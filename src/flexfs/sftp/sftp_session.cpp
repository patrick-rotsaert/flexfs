//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/sftp/sftp_session.h"
#include "flexfs/sftp/sftp_exceptions.h"
#include "flexfs/sftp/ssh_connection.h"
#include "flexfs/sftp/ssh_server_pubkey.h"
#include "flexfs/sftp/ssh_pubkey_hash.h"
#include "flexfs/sftp/ssh_private_key.h"
#include "flexfs/sftp/ssh_public_key.h"
#include "flexfs/sftp/ptr_types.h"
#include "flexfs/core/exceptions.h"
#include "flexfs/core/logging.h"
#include "flexfs/core/log_level.h"
#include "flexfs/core/formatters.h"
#include <fmt/format.h>
#include <cassert>
#include <libssh/callbacks.h>

namespace flexfs {
namespace sftp {

namespace {

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

} // namespace

class session::impl final
{
	i_ssh_api*                      api_;
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
	explicit impl(i_ssh_api*                              api,
	              const options&                          opts,
	              std::shared_ptr<i_ssh_known_hosts>      known_hosts,
	              std::shared_ptr<i_ssh_identity_factory> ssh_identity_factory,
	              std::shared_ptr<i_interruptor>          interruptor)
	    : api_{ api }
	    , interruptor_{ interruptor }
	    , ssh_{}
	    , connection_{}
	    , sftp_{}
	{
		this->api_->ssh_set_log_callback(ssh_logging_callback);

		auto ssh = ssh_session_ptr{ this->api_->ssh_new(), [this](auto s) { this->api_->ssh_free(s); } };
		if (!ssh)
		{
			FLEXFS_THROW(ssh_exception("ssh_new() returned nullptr") << error_opname{ "ssh_new" });
		}

		auto cb                    = ssh_callbacks_struct{};
		cb.userdata                = this;
		cb.connect_status_function = impl::connect_status_callback;
		ssh_callbacks_init(&cb);
		this->api_->ssh_set_callbacks(ssh.get(), &cb);

		// CONNECT
		fslog(debug, "connect host={}, port={}", opts.host, opts.port);

		this->api_->ssh_options_set(ssh.get(), SSH_OPTIONS_HOST, opts.host.c_str());

		if (opts.port)
		{
			this->api_->ssh_options_set(ssh.get(), SSH_OPTIONS_PORT, &(opts.port.value()));
		}

		const auto verbosity = convert_ssh_logging_verbosity(opts.ssh_logging_verbosity);
		this->api_->ssh_options_set(ssh.get(), SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

		auto connection = std::make_unique<ssh_connection>(this->api_, ssh);

		this->interruptor_->throw_if_interrupted();

		// VERIFY HOST KEY
		fslog(debug, "verify host key");

		{
			const auto pubkey = ssh_server_pubkey{ this->api_, ssh.get() };
			const auto hash   = ssh_pubkey_hash{ this->api_, ssh.get(), pubkey.key().get(), SSH_PUBLICKEY_HASH_SHA1 };

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
			if (this->api_->ssh_userauth_none(ssh.get(), nullptr) == SSH_AUTH_ERROR)
			{
				FLEXFS_THROW(ssh_exception("Unexpected error") << error_opname{ "ssh_userauth_none" });
			}

			const auto methods       = this->api_->ssh_userauth_list(ssh.get(), nullptr);
			auto       authenticated = false;

			if (!authenticated && (methods & SSH_AUTH_METHOD_NONE))
			{
				this->interruptor_->throw_if_interrupted();

				fslog(trace, "attempting NONE authentication");
				const auto rc = this->api_->ssh_userauth_none(ssh.get(), opts.user.c_str());
				if (rc == SSH_AUTH_SUCCESS)
				{
					fslog(info, "NONE authentication successful");
					authenticated = true;
				}
				else
				{
					fslog(err, "NONE authentication failed: {}", this->api_->ssh_get_error(ssh.get()));
				}
			}

			if (!authenticated && (methods & SSH_AUTH_METHOD_PUBLICKEY))
			{
				for (const auto& identity : ssh_identity_factory->create())
				{
					this->interruptor_->throw_if_interrupted();

					fslog(debug, "attempting public key authentication with identity '{}'", identity->name);
					const auto pkey   = ssh_private_key{ this->api_, identity->pkey };
					const auto pubkey = ssh_public_key{ this->api_, pkey };
					auto       rc     = this->api_->ssh_userauth_try_publickey(ssh.get(), nullptr, pubkey.key().get());
					if (rc == SSH_AUTH_SUCCESS)
					{
						rc = this->api_->ssh_userauth_publickey(ssh.get(), opts.user.c_str(), pkey.key().get());
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
							fslog(err, "Public key authentication failed: {}", this->api_->ssh_get_error(ssh.get()));
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
						fslog(err, "Public key authentication failed: {}", this->api_->ssh_get_error(ssh.get()));
					}
				}
			}

			if (!authenticated && (methods & SSH_AUTH_METHOD_PASSWORD) && opts.password)
			{
				this->interruptor_->throw_if_interrupted();

				fslog(trace, "attempting password authentication");
				const auto rc = this->api_->ssh_userauth_password(ssh.get(), opts.user.c_str(), opts.password->c_str());
				if (rc == SSH_AUTH_SUCCESS)
				{
					fslog(info, "password authentication successful");
					authenticated = true;
				}
				else
				{
					fslog(err, "Password authentication failed: {}", this->api_->ssh_get_error(ssh.get()));
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

		const auto sftp = sftp_session_ptr{ this->api_->sftp_new(ssh.get()), [this](auto s) { this->api_->sftp_free(s); } };
		if (!sftp)
		{
			FLEXFS_THROW(ssh_exception("sftp_new() returned nullptr") << error_opname{ "sftp_new" });
		}

		{
			const auto rc = this->api_->sftp_init(sftp.get());
			if (rc != SSH_OK)
			{
				FLEXFS_THROW(sftp_exception(ssh.get(), sftp.get()) << error_opname{ "sftp_init" });
			}
		}

		this->connection_ = std::move(connection);
		this->ssh_        = ssh;
		this->sftp_       = sftp;
	}

	~impl()
	{
		this->sftp_.reset();
		this->connection_.reset();
		this->ssh_.reset();
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

session::session(i_ssh_api*                              api,
                 const options&                          opts,
                 std::shared_ptr<i_ssh_known_hosts>      known_hosts,
                 std::shared_ptr<i_ssh_identity_factory> ssh_identity_factory,
                 std::shared_ptr<i_interruptor>          interruptor)
    : pimpl_{ std::make_unique<impl>(api, opts, known_hosts, ssh_identity_factory, interruptor) }
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
