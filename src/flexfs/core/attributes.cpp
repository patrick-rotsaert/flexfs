//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/core/attributes.h"
#include <optional>
#include <sys/types.h>
#include <sys/stat.h>

namespace flexfs {

namespace {

attributes::filetype convert_file_type(std::uint32_t st_mode)
{
	using filetype = attributes::filetype;
	switch (st_mode & S_IFMT)
	{
	case S_IFBLK:
		return filetype::BLOCK;
	case S_IFCHR:
		return filetype::CHAR;
	case S_IFDIR:
		return filetype::DIR;
	case S_IFIFO:
		return filetype::FIFO;
	case S_IFLNK:
		return filetype::LINK;
	case S_IFREG:
		return filetype::FILE;
	case S_IFSOCK:
		return filetype::SOCK;
	default:
		break;
	}
	return filetype::UNKNOWN;
}

attributes::filemodes convert_file_mode(std::uint32_t st_mode)
{
	using filemode = attributes::filemode;
	auto result    = std::set<attributes::filemode>{};
	if (st_mode & S_ISUID)
	{
		result.insert(filemode::SET_UID);
	}
	if (st_mode & S_ISGID)
	{
		result.insert(filemode::SET_GID);
	}
	if (st_mode & S_ISVTX)
	{
		result.insert(filemode::STICKY);
	}
	return result;
}

std::uint32_t convert_file_mode(const attributes::filemodes& mode)
{
	using filemode = attributes::filemode;
	auto st_mode   = std::uint32_t{};
	if (mode.count(filemode::SET_UID))
	{
		st_mode |= S_ISUID;
	}
	if (mode.count(filemode::SET_GID))
	{
		st_mode |= S_ISGID;
	}
	if (mode.count(filemode::STICKY))
	{
		st_mode |= S_ISVTX;
	}
	return st_mode;
}

attributes::fileperms convert_file_perm(std::uint32_t st_mode, std::uint32_t r, std::uint32_t w, std::uint32_t x)
{
	using fileperm = attributes::fileperm;
	auto result    = attributes::fileperms{};
	if (st_mode & r)
	{
		result.insert(fileperm::READ);
	}
	if (st_mode & w)
	{
		result.insert(fileperm::WRITE);
	}
	if (st_mode & x)
	{
		result.insert(fileperm::EXEC);
	}
	return result;
}

std::uint32_t convert_file_perm(const attributes::fileperms& perm, std::uint32_t r, std::uint32_t w, std::uint32_t x)
{
	using fileperm = attributes::fileperm;
	auto st_mode   = std::uint32_t{};
	if (perm.count(fileperm::READ))
	{
		st_mode |= r;
	}
	if (perm.count(fileperm::WRITE))
	{
		st_mode |= w;
	}
	if (perm.count(fileperm::EXEC))
	{
		st_mode |= x;
	}
	return st_mode;
}

} // namespace

bool attributes::is_dir() const
{
	return type == filetype::DIR;
}

bool attributes::is_reg() const
{
	return type == filetype::REG;
}

uint32_t attributes::get_mode() const
{
	return convert_file_perm(uperm, S_IRUSR, S_IWUSR, S_IXUSR) | convert_file_perm(gperm, S_IRGRP, S_IWGRP, S_IXGRP) |
	       convert_file_perm(operm, S_IROTH, S_IWOTH, S_IXOTH) | convert_file_mode(mode);
}

void attributes::set_mode(std::uint32_t st_mode)
{
	//fslog(debug, "st_mode={0:o}", st_mode);
	type  = convert_file_type(st_mode);
	mode  = convert_file_mode(st_mode);
	uperm = convert_file_perm(st_mode, S_IRUSR, S_IWUSR, S_IXUSR);
	gperm = convert_file_perm(st_mode, S_IRGRP, S_IWGRP, S_IXGRP);
	operm = convert_file_perm(st_mode, S_IROTH, S_IWOTH, S_IXOTH);
}

std::string attributes::mode_string() const
{
	char buf[10], *p = buf;
	switch (type)
	{
	case filetype::BLOCK:
		*p++ = 'b';
		break;
	case filetype::CHAR:
		*p++ = 'c';
		break;
	case filetype::DIR:
		*p++ = 'd';
		break;
	case filetype::FIFO:
		*p++ = 'p';
		break;
	case filetype::LINK:
		*p++ = 'l';
		break;
	case filetype::FILE:
		*p++ = '-';
		break;
	case filetype::SOCK:
		*p++ = 's';
		break;
	case filetype::SPECIAL:
		*p++ = 'S';
		break;
	case filetype::UNKNOWN:
		*p++ = '?';
		break;
	}
	*p++ = uperm.count(fileperm::READ) ? 'r' : '-';
	*p++ = uperm.count(fileperm::WRITE) ? 'w' : '-';
	*p++ = uperm.count(fileperm::EXEC) ? 'x' : '-';
	*p++ = gperm.count(fileperm::READ) ? 'r' : '-';
	*p++ = gperm.count(fileperm::WRITE) ? 'w' : '-';
	*p++ = gperm.count(fileperm::EXEC) ? 'x' : '-';
	*p++ = operm.count(fileperm::READ) ? 'r' : '-';
	*p++ = operm.count(fileperm::WRITE) ? 'w' : '-';
	*p++ = operm.count(fileperm::EXEC) ? 'x' : '-';
	return std::string{ buf, sizeof(buf) };
}

std::optional<std::string> attributes::owner_or_uid() const
{
	if (owner)
	{
		return owner.value();
	}
	else if (uid)
	{
		return std::to_string(uid.value());
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<std::string> attributes::group_or_gid() const
{
	if (group)
	{
		return group.value();
	}
	else if (gid)
	{
		return std::to_string(gid.value());
	}
	else
	{
		return std::nullopt;
	}
}

} // namespace flexfs
