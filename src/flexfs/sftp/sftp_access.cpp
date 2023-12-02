#include "flexfs/sftp/sftp_access.h"
#include "flexfs/sftp/sftp_exceptions.h"
#include "flexfs/sftp/sftp_file.h"
#include "flexfs/sftp/sftp_watcher.h"
#include "flexfs/sftp/sftp_session.h"
#include "flexfs/direntry.h"
#include "flexfs/attributes.h"
#include "flexfs/logging.h"
#include "flexfs/formatters.h"
#include <boost/date_time/posix_time/conversion.hpp>
#include <optional>
#include <type_traits>
#include <cassert>
#include <cstddef>
#include <libssh/sftp.h>

namespace flexfs {
namespace sftp {

namespace {

using sftp_attributes_ptr = std::shared_ptr<std::remove_pointer<sftp_attributes>::type>;

attributes::filetype convert_file_type(const sftp_attributes in)
{
	switch (in->type)
	{
	case SSH_FILEXFER_TYPE_REGULAR:
		return attributes::filetype::FILE;
	case SSH_FILEXFER_TYPE_DIRECTORY:
		return attributes::filetype::DIR;
	case SSH_FILEXFER_TYPE_SYMLINK:
		return attributes::filetype::LINK;
	case SSH_FILEXFER_TYPE_SPECIAL:
		return attributes::filetype::SPECIAL;
	case SSH_FILEXFER_TYPE_UNKNOWN:
	default:
		return attributes::filetype::UNKNOWN;
	}
}

boost::posix_time::ptime convert_file_time(uint64_t sec, uint32_t nsec)
{
	return boost::posix_time::from_time_t(sec) + boost::posix_time::microseconds(nsec / 1000);
}

boost::posix_time::ptime convert_file_time(uint32_t sec)
{
	return boost::posix_time::from_time_t(sec);
}

attributes make_attributes(const sftp_attributes in)
{
	fslog(trace, "flags {0:x}h {0:b}b", in->flags);

	auto result = attributes{};

	if (in->flags & SSH_FILEXFER_ATTR_PERMISSIONS)
	{
		result.set_mode(in->permissions);
	}
	else
	{
		result.type = convert_file_type(in);
	}

	if (in->flags & SSH_FILEXFER_ATTR_SIZE)
	{
		result.size = in->size;
	}

	if (in->flags & SSH_FILEXFER_ATTR_UIDGID)
	{
		result.uid = in->uid;
		result.gid = in->gid;
	}

	if (in->flags & SSH_FILEXFER_ATTR_ACMODTIME)
	{
		result.atime = in->flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES ? convert_file_time(in->atime64, in->atime_nseconds)
		                                                             : convert_file_time(in->atime);
		result.mtime = in->flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES ? convert_file_time(in->mtime64, in->mtime_nseconds)
		                                                             : convert_file_time(in->mtime);
	}
	else
	{
		if (in->flags & SSH_FILEXFER_ATTR_ACCESSTIME)
		{
			result.atime = in->flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES ? convert_file_time(in->atime64, in->atime_nseconds)
			                                                             : convert_file_time(in->atime);
		}

		if (in->flags & SSH_FILEXFER_ATTR_MODIFYTIME)
		{
			result.mtime = in->flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES ? convert_file_time(in->mtime64, in->mtime_nseconds)
			                                                             : convert_file_time(in->mtime);
		}
	}

	if (in->flags & SSH_FILEXFER_ATTR_CREATETIME)
	{
		result.ctime = convert_file_time(in->createtime, in->createtime_nseconds);
	}

	if (in->owner)
	{
		result.owner = in->owner;
	}

	if (in->group)
	{
		result.group = in->group;
	}

	return result;
}

direntry make_direntry(const sftp_attributes a)
{
	assert(a != nullptr);
	auto entry = direntry{};
	entry.name = a->name;
	entry.attr = make_attributes(a);
	return entry;
}

class directory_reader
{
	std::shared_ptr<session> session_;
	fspath                   path_;
	sftp_dir                 dir_;
	struct dirent*           entry_;

public:
	explicit directory_reader(std::shared_ptr<session> session, const fspath& path)
	    : session_{ session }
	    , path_{ path }
	{
		fslog(trace, "sftp_opendir path={}", path);
		this->dir_ = sftp_opendir(session->sftp(), path.string().c_str());
		fslog(trace, "dir {}", fmt::ptr(this->dir_));

		if (!this->dir_)
		{
			FLEXFS_THROW(sftp_exception(this->session_) << error_opname{ "sftp_opendir" } << error_path{ path });
		}
	}

	~directory_reader() noexcept
	{
		fslog(trace, "sftp_closedir {}", fmt::ptr(this->dir_));
		sftp_closedir(this->dir_);
	}

	std::optional<direntry> read()
	{
		fslog(trace, "sftp_readdir {}", fmt::ptr(this->dir_));
		const auto attrib = sftp_readdir(this->session_->sftp(), this->dir_);
		if (attrib)
		{
			sftp_attributes_ptr guard{ attrib, sftp_attributes_free };
			return make_direntry(attrib);
		}
		else
		{
			if (sftp_dir_eof(this->dir_))
			{
				return std::nullopt;
			}
			else
			{
				FLEXFS_THROW(sftp_exception(this->session_) << error_opname{ "sftp_readdir" } << error_path{ this->path_ });
			}
		}
	}
};

} // namespace

class access::impl
{
	access&                        owner_;
	std::shared_ptr<i_interruptor> interruptor_;
	std::shared_ptr<session>       session_;
	std::uint32_t                  watcher_scan_interval_ms_;

public:
	explicit impl(access&                                 owner,
	              const options&                          opts,
	              std::shared_ptr<i_ssh_known_hosts>      known_hosts,
	              std::shared_ptr<i_ssh_identity_factory> ssh_identity_factory,
	              std::shared_ptr<i_interruptor>          interruptor)
	    : owner_{ owner }
	    , interruptor_{ interruptor }
	    , session_{ std::make_shared<session>(opts, known_hosts, ssh_identity_factory, interruptor) }
	    , watcher_scan_interval_ms_{ opts.watcher_scan_interval_ms }
	{
		fslog(trace, "sftp access: host={}, port={}, user={}", opts.host, opts.port, opts.user);
	}

	std::vector<direntry> ls(const fspath& dir)
	{
		this->interruptor_->throw_if_interrupted();

		auto result = std::vector<direntry>{};
		auto dr     = directory_reader{ this->session_, dir };
		while (auto entry = dr.read())
		{
			this->interruptor_->throw_if_interrupted();
			if (entry->attr.type == attributes::filetype::LINK)
			{
				auto path = dir / entry->name;
				fslog(trace, "sftp_stat path={}", path);
				const auto attrib = sftp_stat(this->session_->sftp(), path.string().c_str());
				if (attrib)
				{
					const auto guard = sftp_attributes_ptr{ attrib, sftp_attributes_free };
					entry->link      = make_attributes(attrib);
				}
				else
				{
					// stat fails with ENOENT if a dead link is encountered, which is non-critical
					const auto err = sftp_get_error(this->session_->sftp());
					if (err != SSH_FX_NO_SUCH_FILE)
					{
						FLEXFS_THROW(sftp_exception(this->session_) << error_opname{ "sftp_stat" } << error_path{ path });
					}
				}
			}
			result.push_back(std::move(entry.value()));
		}
		return result;
	}

	bool exists(const fspath& path)
	{
		this->interruptor_->throw_if_interrupted();

		fslog(trace, "sftp_stat path={}", path);
		const auto attrib = sftp_stat(this->session_->sftp(), path.string().c_str());
		if (attrib)
		{
			sftp_attributes_free(attrib);
			return true;
		}
		else
		{
			int err = sftp_get_error(this->session_->sftp());
			if (err == SSH_FX_NO_SUCH_FILE)
			{
				return false;
			}
			else
			{
				FLEXFS_THROW(sftp_exception(this->session_) << error_opname{ "sftp_stat" } << error_path{ path });
			}
		}
	}

	std::optional<attributes> try_stat(const fspath& path)
	{
		this->interruptor_->throw_if_interrupted();

		fslog(trace, "sftp_stat path={}", path);
		const auto attrib = sftp_stat(this->session_->sftp(), path.string().c_str());
		if (attrib)
		{
			const auto guard = sftp_attributes_ptr{ attrib, sftp_attributes_free };
			return make_attributes(attrib);
		}
		else
		{
			const auto err = sftp_get_error(this->session_->sftp());
			if (err == SSH_FX_NO_SUCH_FILE)
			{
				return std::nullopt;
			}
			else
			{
				FLEXFS_THROW(sftp_exception(this->session_) << error_opname{ "sftp_stat" } << error_path{ path });
			}
		}
	}

	attributes stat(const fspath& path)
	{
		this->interruptor_->throw_if_interrupted();

		fslog(trace, "sftp_stat path={}", path);
		const auto attrib = sftp_stat(this->session_->sftp(), path.string().c_str());
		if (attrib)
		{
			const auto guard = sftp_attributes_ptr{ attrib, sftp_attributes_free };
			return make_attributes(attrib);
		}
		else
		{
			FLEXFS_THROW(sftp_exception(this->session_) << error_opname{ "sftp_stat" } << error_path{ path });
		}
	}

	attributes lstat(const fspath& path)
	{
		this->interruptor_->throw_if_interrupted();

		fslog(trace, "sftp_lstat path={}", path);
		const auto attrib = sftp_lstat(this->session_->sftp(), path.string().c_str());
		if (attrib)
		{
			const auto guard = sftp_attributes_ptr{ attrib, sftp_attributes_free };
			return make_attributes(attrib);
		}
		else
		{
			FLEXFS_THROW(sftp_exception(this->session_) << error_opname{ "sftp_lstat" } << error_path{ path });
		}
	}

	void remove(const fspath& path)
	{
		this->interruptor_->throw_if_interrupted();

		fslog(trace, "sftp_unlink path={}", path);
		if (sftp_unlink(this->session_->sftp(), path.string().c_str()) < 0)
		{
			FLEXFS_THROW(sftp_exception(this->session_) << error_opname{ "sftp_unlink" } << error_path{ path });
		}
	}

	void mkdir(const fspath& path, bool parents)
	{
		this->interruptor_->throw_if_interrupted();

		fslog(trace, "sftp_mkdir path={}", path);
		if (sftp_mkdir(this->session_->sftp(), path.string().c_str(), 0777) < 0)
		{
			if (parents && path.has_parent_path() && sftp_get_error(this->session_->sftp()) == SSH_FX_NO_SUCH_FILE)
			{
				this->mkdir(path.parent_path(), true);
				this->mkdir(path, false);
				return;
			}
			FLEXFS_THROW(sftp_exception(this->session_) << error_opname{ "sftp_mkdir" } << error_path{ path });
		}
	}

	void rename(const fspath& oldpath, const fspath& newpath)
	{
		this->interruptor_->throw_if_interrupted();

		fslog(trace, "sftp_rename oldpath={} newpath={}", oldpath, newpath);
		if (sftp_rename(this->session_->sftp(), oldpath.string().c_str(), newpath.string().c_str()) < 0)
		{
			FLEXFS_THROW(sftp_exception(this->session_)
			             << error_opname{ "sftp_rename" } << error_oldpath{ oldpath } << error_newpath{ newpath });
		}
	}

	std::shared_ptr<i_file> open(const fspath& path, int flags, mode_t mode)
	{
		this->interruptor_->throw_if_interrupted();

		fslog(trace, "sftp_open path={} flags={:o} mode={:o}", path, flags, mode);
		const auto fd = sftp_open(this->session_->sftp(), path.string().c_str(), flags, mode);
		fslog(trace, "fd {}", static_cast<void*>(fd));
		if (fd == nullptr)
		{
			FLEXFS_THROW(sftp_exception(this->session_) << error_opname{ "sftp_open" } << error_path{ path });
		}
		else
		{
			return std::make_shared<file>(fd, path, this->session_, this->interruptor_);
		}
	}

	std::shared_ptr<i_watcher> create_watcher(const fspath& dir, int cancelfd)
	{
		(void)cancelfd;
		return std::make_shared<watcher>(dir, this->watcher_scan_interval_ms_, this->owner_.shared_from_this(), this->interruptor_);
	}
};

access::access(const options&                          opts,
               std::shared_ptr<i_ssh_known_hosts>      known_hosts,
               std::shared_ptr<i_ssh_identity_factory> ssh_identity_factory,
               std::shared_ptr<i_interruptor>          interruptor)
    : pimpl_{ std::make_unique<impl>(*this, opts, known_hosts, ssh_identity_factory, interruptor) }
{
}

access::~access() noexcept
{
}

bool access::is_remote() const
{
	return true;
}

std::vector<direntry> access::ls(const fspath& dir)
{
	return this->pimpl_->ls(dir);
}

bool access::exists(const fspath& path)
{
	return this->pimpl_->exists(path);
}

std::optional<attributes> access::try_stat(const fspath& path)
{
	return this->pimpl_->try_stat(path);
}

attributes access::stat(const fspath& path)
{
	return this->pimpl_->stat(path);
}

attributes access::lstat(const fspath& path)
{
	return this->pimpl_->lstat(path);
}

void access::remove(const fspath& path)
{
	return this->pimpl_->remove(path);
}

void access::mkdir(const fspath& path, bool parents)
{
	return this->pimpl_->mkdir(path, parents);
}

void access::rename(const fspath& oldpath, const fspath& newpath)
{
	return this->pimpl_->rename(oldpath, newpath);
}

std::shared_ptr<i_file> access::open(const fspath& path, int flags, mode_t mode)
{
	return this->pimpl_->open(path, flags, mode);
}

std::shared_ptr<i_watcher> access::create_watcher(const fspath& dir, int cancelfd)
{
	return this->pimpl_->create_watcher(dir, cancelfd);
}

} // namespace sftp
} // namespace flexfs