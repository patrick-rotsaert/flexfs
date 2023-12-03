#include "flexfs/local/local_access.h"
#include "flexfs/local/local_file.h"
#include "flexfs/local/local_watcher.h"
#include "flexfs/core/logging.h"
#include "flexfs/core/formatters.h"
#include "flexfs/core/direntry.h"
#include "flexfs/core/exceptions.h"

#include <boost/filesystem/operations.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/scoped_array.hpp>
#include <optional>

//#define BOOST_STACKTRACE_USE_BACKTRACE
#include <boost/stacktrace.hpp>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#ifdef BOOST_WINDOWS_API
#include <io.h>
#endif

#ifdef BOOST_WINDOWS_API
#define c_open(pathname, flags, mode) ::_open(pathname, flags, mode)
#else
#define c_open(pathname, flags, mode) ::open(pathname, flags, mode)
#endif

#define THROW_PATH_OP_ERROR(path, opname) FLEXFS_THROW(system_exception{} << error_path{ path } << error_opname{ opname })

namespace flexfs {
namespace local {

namespace {

//boost::posix_time::ptime convert_file_time(const struct timespec& ts)
//{
//	return boost::posix_time::from_time_t(ts.tv_sec) + boost::posix_time::microseconds(ts.tv_nsec / 1000);
//}

//std::optional<std::string> get_user_name(uid_t uid)
//{
//	auto bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
//	if (bufsize < 0)
//	{
//		bufsize = 256;
//	}
//	struct passwd             pwd, *ppwd = nullptr;
//	boost::scoped_array<char> buf(new char[bufsize]);
//	auto                      rc = getpwuid_r(uid, &pwd, buf.get(), bufsize, &ppwd);
//	if (rc || ppwd == nullptr || ppwd->pw_name == nullptr)
//	{
//		fslog(err,"getpwuid_r({}) failed: {}", uid, rc);
//		return std::nullopt;
//	}
//	else
//	{
//		return std::string(ppwd->pw_name);
//	}
//}

//std::optional<std::string> get_group_name(gid_t gid)
//{
//	auto bufsize = sysconf(_SC_GETGR_R_SIZE_MAX);
//	if (bufsize < 0)
//	{
//		bufsize = 256;
//	}
//	struct group              grp, *pgrp = nullptr;
//	boost::scoped_array<char> buf(new char[bufsize]);
//	auto                      rc = getgrgid_r(gid, &grp, buf.get(), bufsize, &pgrp);
//	if (rc || pgrp == nullptr || pgrp->gr_name == nullptr)
//	{
//		fslog(err,"getgrgid_r({}) failed: {}", gid, rc);
//		return std::nullopt;
//	}
//	else
//	{
//		return std::string(pgrp->gr_name);
//	}
//}

attributes make_attributes(const fspath& path, const boost::filesystem::file_status& st)
{
	auto result = attributes{};

	result.set_mode(static_cast<std::uint32_t>(st.permissions()));

	switch (st.type())
	{
	case boost::filesystem::regular_file:
		result.type = attributes::filetype::REG;
		break;
	case boost::filesystem::directory_file:
		result.type = attributes::filetype::DIR;
		break;
	case boost::filesystem::symlink_file:
		result.type = attributes::filetype::LNK;
		break;
	case boost::filesystem::block_file:
		result.type = attributes::filetype::BLOCK;
		break;
	case boost::filesystem::character_file:
		result.type = attributes::filetype::CHAR;
		break;
	case boost::filesystem::fifo_file:
		result.type = attributes::filetype::FIFO;
		break;
	case boost::filesystem::socket_file:
		result.type = attributes::filetype::SOCK;
		break;
	default:
		result.type = attributes::filetype::UNKNOWN;
		break;
	}

	{
		auto       ec    = boost::system::error_code{};
		const auto mtime = boost::filesystem::last_write_time(path, ec);
		if (!ec)
		{
			result.mtime = boost::posix_time::from_time_t(mtime);
		}
	}
	{
		auto       ec   = boost::system::error_code{};
		const auto size = boost::filesystem::file_size(path, ec);
		if (!ec)
		{
			result.size = size;
		}
	}

	//	out.uid   = in.st_uid;
	//	out.gid   = in.st_gid;
	//	out.atime = convert_file_time(in.st_atim);
	//	out.ctime = convert_file_time(in.st_ctim);
	//	out.owner = get_user_name(in.st_uid);
	//	out.group = get_group_name(in.st_gid);

	return result;
}

direntry make_direntry(const fspath& path, const boost::filesystem::file_status& st)
{
	auto result = direntry{};

	result.name = path.filename().string();
	result.attr = make_attributes(path, st);

	if (st.type() == boost::filesystem::symlink_file)
	{
		fslog(trace, "read_symlink path={}", path);
		auto ec               = boost::system::error_code{};
		result.symlink_target = boost::filesystem::read_symlink(path, ec);
		if (ec)
		{
			FLEXFS_THROW(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "read_symlink" });
		}
	}

	return result;
}

direntry make_direntry(const boost::filesystem::directory_entry& e)
{
	return make_direntry(e.path(), e.symlink_status());
}

} // namespace

access::access(std::shared_ptr<i_interruptor> interruptor)
    : interruptor_{ interruptor }
{
	//fslog(trace,"local access\n{}", boost::stacktrace::stacktrace());
	fslog(trace, "local access");
}

bool access::is_remote() const
{
	return false;
}

std::vector<direntry> access::ls(const fspath& dir)
{
	this->interruptor_->throw_if_interrupted();
	auto result = std::vector<direntry>{};
	for (boost::filesystem::directory_iterator it(dir), end; it != end; ++it)
	{
		this->interruptor_->throw_if_interrupted();
		result.push_back(make_direntry(*it));
	}
	return result;
}

bool access::exists(const fspath& path)
{
	this->interruptor_->throw_if_interrupted();
	fslog(trace, "exists path={}", path);
	return boost::filesystem::exists(path);
}

std::optional<attributes> access::try_stat(const fspath& path)
{
	this->interruptor_->throw_if_interrupted();

	fslog(trace, "try_stat path={}", path);
	auto       ec = boost::system::error_code{};
	const auto st = boost::filesystem::directory_entry(path).status(ec);
	if (ec == boost::system::errc::no_such_file_or_directory)
	{
		return std::nullopt;
	}
	else if (ec)
	{
		FLEXFS_THROW(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "status" });
	}

	return make_attributes(path, st);
}

attributes access::stat(const fspath& path)
{
	this->interruptor_->throw_if_interrupted();

	fslog(trace, "stat path={}", path);
	auto       ec = boost::system::error_code{};
	const auto st = boost::filesystem::directory_entry(path).status(ec);
	if (ec)
	{
		FLEXFS_THROW(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "status" });
	}

	return make_attributes(path, st);
}

attributes access::lstat(const fspath& path)
{
	this->interruptor_->throw_if_interrupted();

	fslog(trace, "lstat path={}", path);
	auto       ec = boost::system::error_code{};
	const auto st = boost::filesystem::directory_entry(path).symlink_status(ec);
	if (ec)
	{
		FLEXFS_THROW(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "symlink_status" });
	}

	return make_attributes(path, st);
}

void access::remove(const fspath& path)
{
	this->interruptor_->throw_if_interrupted();

	fslog(trace, "remove path={}", path);
	auto ec = boost::system::error_code{};
	boost::filesystem::remove(path, ec);
	if (ec.failed())
	{
		FLEXFS_THROW(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "remove" });
	}
}

void access::mkdir(const fspath& path, bool parents)
{
	this->interruptor_->throw_if_interrupted();

	if (parents)
	{
		fslog(trace, "create_directories path={}", path);
		auto ec = boost::system::error_code{};
		boost::filesystem::create_directories(path, ec);
		if (ec.failed())
		{
			FLEXFS_THROW(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "create_directories" });
		}
	}
	else
	{
		fslog(trace, "create_directory path={}", path);
		auto ec = boost::system::error_code{};
		boost::filesystem::create_directory(path, ec);
		if (ec.failed())
		{
			FLEXFS_THROW(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "create_directory" });
		}
	}
}

void access::rename(const fspath& oldpath, const fspath& newpath)
{
	this->interruptor_->throw_if_interrupted();

	fslog(trace, "rename oldpath={} newpath={}", oldpath, newpath);
	auto ec = boost::system::error_code{};
	boost::filesystem::rename(oldpath, newpath, ec);
	if (ec.failed())
	{
		FLEXFS_THROW(system_exception(std::error_code{ ec })
		             << error_oldpath{ oldpath } << error_newpath{ newpath } << error_opname{ "rename" });
	}
}

std::shared_ptr<i_file> access::open(const fspath& path, int flags, mode_t mode)
{
	this->interruptor_->throw_if_interrupted();

	fslog(trace, "open path={} flags={:o} mode={:o}", path, flags, mode);
	const auto fd = c_open(path.string().c_str(), flags, mode);
	fslog(trace, "fd {}", fd);
	if (fd == -1)
	{
		THROW_PATH_OP_ERROR(path, "open");
	}
	else
	{
		return std::make_shared<file>(fd, path, this->interruptor_);
	}
}

std::shared_ptr<i_watcher> access::create_watcher(const fspath& dir, int cancelfd)
{
	return std::make_shared<watcher>(dir, cancelfd);
}

direntry access::get_direntry(const fspath& path)
{
	return make_direntry(boost::filesystem::directory_entry{ path });
}

} // namespace local
} // namespace flexfs
