#include "flexfs/local/make_attributes.h"
#include <boost/filesystem/operations.hpp>

//#include <dirent.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <errno.h>
//#include <unistd.h>
//#include <pwd.h>
//#include <grp.h>

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

} // namespace

attributes::filetype make_filetype(boost::filesystem::file_type type)
{
	switch (type)
	{
	case boost::filesystem::regular_file:
		return attributes::filetype::REG;
	case boost::filesystem::directory_file:
		return attributes::filetype::DIR;
	case boost::filesystem::symlink_file:
		return attributes::filetype::LNK;
	case boost::filesystem::block_file:
		return attributes::filetype::BLOCK;
	case boost::filesystem::character_file:
		return attributes::filetype::CHAR;
	case boost::filesystem::fifo_file:
		return attributes::filetype::FIFO;
	case boost::filesystem::socket_file:
		return attributes::filetype::SOCK;
	default:
		break;
	}
	return attributes::filetype::UNKNOWN;
}

attributes make_attributes(const fspath& path, const boost::filesystem::file_status& st)
{
	auto result = attributes{};

	result.set_mode(static_cast<mode_t>(st.permissions()));

	// `type` must be set *after* calling attributes::set_mode(...), because that method would
	// otherwise overwrite the `type` member.
	result.type = make_filetype(st.type());

	{
		auto       ec    = boost::system::error_code{};
		const auto mtime = boost::filesystem::last_write_time(path, ec);
		if (!ec)
		{
			result.mtime = std::chrono::system_clock::from_time_t(mtime);
		}
	}

	{
		auto       ec    = boost::system::error_code{};
		const auto ctime = boost::filesystem::creation_time(path, ec);
		if (!ec)
		{
			result.ctime = std::chrono::system_clock::from_time_t(ctime);
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

} // namespace local
} // namespace flexfs
