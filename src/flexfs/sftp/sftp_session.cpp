//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/sftp/sftp_session.h"
#include "flexfs/sftp/sftp_exceptions.h"
#include "flexfs/core/exceptions.h"
#include "flexfs/core/logging.h"
#include "flexfs/core/log_level.h"
#include "flexfs/core/formatters.h"
#include <fmt/format.h>
#include <type_traits>
#include <cassert>
#include <libssh/callbacks.h>

namespace flexfs {
namespace sftp {

namespace {

using ssh_key_ptr      = std::shared_ptr<std::remove_pointer<ssh_key>::type>;
using ssh_session_ptr  = std::shared_ptr<std::remove_pointer<ssh_session>::type>;
using sftp_session_ptr = std::shared_ptr<std::remove_pointer<sftp_session>::type>;

void ssh_logging_callback(int priority, const char* function, const char* buffer, void* /*userdata*/)
{
	auto& logger = logging::logger;
	if (!logger)
	{
		return;
	}

	auto lvl = log_level{};
	switch (priority)
	{
	case SSH_LOG_NONE:
		lvl = log_level::off;
		break;
	case SSH_LOG_WARN:
		lvl = log_level::warn;
		break;
	case SSH_LOG_INFO:
		lvl = log_level::info;
		break;
	case SSH_LOG_DEBUG:
		lvl = log_level::debug;
		break;
	case SSH_LOG_TRACE:
		lvl = log_level::trace;
		break;
	default:
		return;
	}

	logger->log_message(
	    std::chrono::system_clock::now(), boost::source_location{ nullptr, 0, function }, lvl, fmt::format("[ssh] {}", buffer));
}

int convert_ssh_logging_verbosity(options::ssh_log_level in)
{
	switch (in)
	{
	case options::ssh_log_level::NOLOG:
		return SSH_LOG_NOLOG;
	case options::ssh_log_level::WARNING:
		return SSH_LOG_WARNING;
	case options::ssh_log_level::PROTOCOL:
		return SSH_LOG_PROTOCOL;
	case options::ssh_log_level::PACKET:
		return SSH_LOG_PACKET;
	case options::ssh_log_level::FUNCTIONS:
		return SSH_LOG_FUNCTIONS;
	}
	FLEXFS_THROW(should_not_happen_exception{});
}

class ssh_connection
{
	ssh_session_ptr session_;

public:
	explicit ssh_connection(ssh_session_ptr session)
	    : session_{ session }
	{
		if (ssh_connect(this->session_.get()) != SSH_OK)
		{
			FLEXFS_THROW(ssh_exception(this->session_.get()) << error_opname{ "ssh_connect" });
		}
	}

	~ssh_connection() noexcept
	{
		ssh_disconnect(this->session_.get());
	}
};

class ssh_server_pubkey
{
	ssh_key_ptr key_;

public:
	explicit ssh_server_pubkey(ssh_session_ptr session)
	{
		ssh_key    key = nullptr;
		const auto rc  = ssh_get_server_publickey(session.get(), &key);
		if (rc < 0)
		{
			FLEXFS_THROW(ssh_exception(session.get()) << error_opname{ "ssh_get_server_publickey" });
		}
		this->key_ = ssh_key_ptr{ key, ssh_key_free };
	}

	ssh_key_ptr key() const
	{
		return this->key_;
	}
};

class ssh_pubkey_hash
{
	std::string hash_;

public:
	explicit ssh_pubkey_hash(ssh_session_ptr session, ssh_key_ptr key, ssh_publickey_hash_type type)
	{
		unsigned char* hash = nullptr;
		auto           hlen = size_t{};
		const auto     rc   = ssh_get_publickey_hash(key.get(), type, &hash, &hlen);
		if (rc < 0)
		{
			FLEXFS_THROW(ssh_exception(session.get()) << error_opname{ "ssh_get_publickey_hash" });
		}
		assert(hash);
		auto hexa = ssh_get_hexa(hash, hlen);
		assert(hexa);
		ssh_clean_pubkey_hash(&hash);
		this->hash_.assign(hexa);
		ssh_string_free_char(hexa);
	}

	const std::string hash() const
	{
		return this->hash_;
	}
};

class ssh_private_key
{
	ssh_key_ptr key_;

public:
	explicit ssh_private_key(const std::string& b64)
	{
		ssh_key    key = nullptr;
		const auto rc  = ssh_pki_import_privkey_base64(b64.c_str(), nullptr, nullptr, nullptr, &key);
		if (rc != SSH_OK)
		{
			FLEXFS_THROW(ssh_exception{} << error_opname{ "ssh_pki_import_privkey_base64" });
		}
		const auto keyptr = ssh_key_ptr{ key, ssh_key_free };
		if (!ssh_key_is_private(key))
		{
			FLEXFS_THROW(ssh_exception{} << error_mesg{ "Not a private key" } << error_opname{ "ssh_key_is_private" });
		}
		this->key_ = keyptr;
	}

	ssh_key_ptr key() const
	{
		return this->key_;
	}
};

class ssh_public_key
{
	ssh_key_ptr key_;

public:
	explicit ssh_public_key(const ssh_private_key& pkey)
	{
		auto       key = ssh_key{ nullptr };
		const auto rc  = ssh_pki_export_privkey_to_pubkey(pkey.key().get(), &key);
		if (rc != SSH_OK)
		{
			FLEXFS_THROW(ssh_exception{} << error_opname{ "ssh_pki_export_privkey_to_pubkey" });
		}
		this->key_ = ssh_key_ptr{ key, ssh_key_free };
		assert(ssh_key_is_public(key));
	}

	ssh_key_ptr key() const
	{
		return this->key_;
	}
};

} // namespace

class session::impl final
{
	std::shared_ptr<i_interruptor>  interruptor_;
	ssh_session_ptr                 ssh_;
	std::unique_ptr<ssh_connection> connection_;
	sftp_session_ptr                sftp_;

	static void connect_status_callback(void* userdata, float status)
	{
		//fslog(trace,"connect status {}", status);
		(void)status;

		auto self = reinterpret_cast<impl*>(userdata);
		self->interruptor_->throw_if_interrupted();
	}

public:
	explicit impl(const options&                          opts,
	              std::shared_ptr<i_ssh_known_hosts>      known_hosts,
	              std::shared_ptr<i_ssh_identity_factory> ssh_identity_factory,
	              std::shared_ptr<i_interruptor>          interruptor)
	    : interruptor_{ interruptor }
	    , ssh_{}
	    , connection_{}
	    , sftp_{}
	{
		ssh_set_log_callback(ssh_logging_callback);

		ssh_session_ptr ssh(ssh_new(), ssh_free);
		if (!ssh)
		{
			FLEXFS_THROW(ssh_exception("ssh_new() returned nullptr") << error_opname{ "ssh_new" });
		}

		auto cb                    = ssh_callbacks_struct{};
		cb.userdata                = this;
		cb.connect_status_function = impl::connect_status_callback;
		ssh_callbacks_init(&cb);
		ssh_set_callbacks(ssh.get(), &cb);

		// CONNECT
		fslog(debug, "connect host={}, port={}", opts.host, opts.port);

		ssh_options_set(ssh.get(), SSH_OPTIONS_HOST, opts.host.c_str());

		if (opts.port)
		{
			ssh_options_set(ssh.get(), SSH_OPTIONS_PORT, &(opts.port.value()));
		}

		const auto verbosity = convert_ssh_logging_verbosity(opts.ssh_logging_verbosity);
		ssh_options_set(ssh.get(), SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

		auto connection = std::make_unique<ssh_connection>(ssh);

		this->interruptor_->throw_if_interrupted();

		// VERIFY HOST KEY
		fslog(debug, "verify host key");

		{
			const auto pubkey = ssh_server_pubkey{ ssh };
			const auto hash   = ssh_pubkey_hash{ ssh, pubkey.key(), SSH_PUBLICKEY_HASH_SHA1 };

			switch (known_hosts->verify(opts.host, hash.hash()))
			{
			case i_ssh_known_hosts::result::KNOWN:
				fslog(debug, "host is known");
				break;
			case i_ssh_known_hosts::result::UNKNOWN:
				fslog(debug, "host is unknown");
				if (opts.allow_unknown_host_key)
				{
					known_hosts->persist(opts.host, hash.hash());
				}
				else
				{
					FLEXFS_THROW(ssh_host_key_unknown("Unknown SSH host")
					             << ssh_host_key_unknown::ssh_host(opts.host) << ssh_host_key_unknown::ssh_host_pubkey_hash(hash.hash()));
				}
				break;
			case i_ssh_known_hosts::result::CHANGED:
				fslog(warn, "host key changed");
				if (opts.allow_changed_host_key)
				{
					known_hosts->persist(opts.host, hash.hash());
				}
				else
				{
					FLEXFS_THROW(ssh_host_key_changed("SSH host key changed")
					             << ssh_host_key_changed::ssh_host(opts.host) << ssh_host_key_changed::ssh_host_pubkey_hash(hash.hash()));
				}
				break;
			}
		}

		this->interruptor_->throw_if_interrupted();

		// AUTHENTICATE
		fslog(debug, "authenticate user {}", opts.user);

		{
			// NOTE: ssh_userauth_list requires the function ssh_userauth_none() to be called before the methods are available.
			if (ssh_userauth_none(ssh.get(), nullptr) == SSH_AUTH_ERROR)
			{
				FLEXFS_THROW(ssh_exception("Unexpected error") << error_opname{ "ssh_userauth_none" });
			}

			const auto methods       = ssh_userauth_list(ssh.get(), nullptr);
			auto       authenticated = false;

			if (!authenticated && (methods & SSH_AUTH_METHOD_NONE))
			{
				this->interruptor_->throw_if_interrupted();

				fslog(trace, "attempting NONE authentication");
				const auto rc = ssh_userauth_none(ssh.get(), opts.user.c_str());
				if (rc == SSH_AUTH_SUCCESS)
				{
					fslog(info, "NONE authentication successful");
					authenticated = true;
				}
				else
				{
					fslog(err, "NONE authentication failed: {}", ssh_get_error(ssh.get()));
				}
			}

			if (!authenticated && (methods & SSH_AUTH_METHOD_PUBLICKEY))
			{
				for (const auto& identity : ssh_identity_factory->create())
				{
					this->interruptor_->throw_if_interrupted();

					fslog(debug, "attempting public key authentication with identity '{}'", identity->name);
					const auto pkey   = ssh_private_key{ identity->pkey };
					const auto pubkey = ssh_public_key{ pkey };
					auto       rc     = ssh_userauth_try_publickey(ssh.get(), nullptr, pubkey.key().get());
					if (rc == SSH_AUTH_SUCCESS)
					{
						rc = ssh_userauth_publickey(ssh.get(), opts.user.c_str(), pkey.key().get());
						if (rc == SSH_AUTH_SUCCESS)
						{
							fslog(info, "public key authentication successful with identity '{}'", identity->name);
							authenticated = true;
						}
						else if (rc == SSH_AUTH_ERROR)
						{
							FLEXFS_THROW(ssh_auth_exception(ssh.get()) << error_opname{ "ssh_userauth_publickey" });
						}
						else
						{
							// do not throw, this error is probably not fatal and another auth method might still succeed
							fslog(err, "Public key authentication failed: {}", ssh_get_error(ssh.get()));
						}
					}
					else if (rc == SSH_AUTH_DENIED)
					{
						fslog(debug, "server refused public key");
					}
					else if (rc == SSH_AUTH_ERROR)
					{
						FLEXFS_THROW(ssh_auth_exception(ssh.get()) << error_opname{ "ssh_userauth_try_publickey" });
					}
					else
					{
						// do not throw, this error is probably not fatal and another auth method might still succeed
						fslog(err, "Public key authentication failed: {}", ssh_get_error(ssh.get()));
					}
				}
			}

			if (!authenticated && (methods & SSH_AUTH_METHOD_PASSWORD) && opts.password)
			{
				this->interruptor_->throw_if_interrupted();

				fslog(trace, "attempting password authentication");
				const auto rc = ssh_userauth_password(ssh.get(), opts.user.c_str(), opts.password->c_str());
				if (rc == SSH_AUTH_SUCCESS)
				{
					fslog(info, "password authentication successful");
					authenticated = true;
				}
				else
				{
					fslog(err, "Password authentication failed: {}", ssh_get_error(ssh.get()));
				}
			}

			if (!authenticated)
			{
				FLEXFS_THROW(ssh_auth_exception("None of the offered authentication methods were successful"));
			}
		}

		this->interruptor_->throw_if_interrupted();

		// SFTP session
		fslog(debug, "create sftp session");

		const auto sftp = sftp_session_ptr{ sftp_new(ssh.get()), sftp_free };
		if (!sftp)
		{
			FLEXFS_THROW(ssh_exception("sftp_new() returned nullptr") << error_opname{ "sftp_new" });
		}

		{
			const auto rc = sftp_init(sftp.get());
			if (rc != SSH_OK)
			{
				FLEXFS_THROW(sftp_exception(ssh.get(), sftp.get()) << error_opname{ "sftp_init" });
			}
		}

		this->connection_ = std::move(connection);
		this->ssh_        = ssh;
		this->sftp_       = sftp;
	}

	ssh_session ssh() const
	{
		return this->ssh_.get();
	}

	sftp_session sftp() const
	{
		return this->sftp_.get();
	}
};

session::session(const options&                          opts,
                 std::shared_ptr<i_ssh_known_hosts>      known_hosts,
                 std::shared_ptr<i_ssh_identity_factory> ssh_identity_factory,
                 std::shared_ptr<i_interruptor>          interruptor)
    : pimpl_{ std::make_unique<impl>(opts, known_hosts, ssh_identity_factory, interruptor) }
{
}

session::~session() noexcept
{
}

ssh_session session::ssh() const
{
	return this->pimpl_->ssh();
}

sftp_session session::sftp() const
{
	return this->pimpl_->sftp();
}

} // namespace sftp
} // namespace flexfs
