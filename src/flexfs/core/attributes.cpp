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

attributes::filetype convert_file_type(mode_t mode)
{
	using filetype = attributes::filetype;
	switch (mode & S_IFMT)
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

mode_t convert_file_type(attributes::filetype type)
{
	using filetype = attributes::filetype;
	switch (type)
	{
	case filetype::BLOCK:
		return S_IFBLK;
	case filetype::CHAR:
		return S_IFCHR;
	case filetype::DIR:
		return S_IFDIR;
	case filetype::FIFO:
		return S_IFIFO;
	case filetype::LINK:
		return S_IFLNK;
	case filetype::FILE:
		return S_IFREG;
	case filetype::SOCK:
		return S_IFSOCK;
	default:
		break;
	}
	return 0;
}

attributes::filemodes convert_file_mode(mode_t mode)
{
	using filemode = attributes::filemode;
	auto result    = std::set<attributes::filemode>{};
	if (mode & S_ISUID)
	{
		result.insert(filemode::SET_UID);
	}
	if (mode & S_ISGID)
	{
		result.insert(filemode::SET_GID);
	}
	if (mode & S_ISVTX)
	{
		result.insert(filemode::STICKY);
	}
	return result;
}

mode_t convert_file_mode(const attributes::filemodes& modes)
{
	using filemode = attributes::filemode;
	auto mode      = mode_t{};
	if (modes.count(filemode::SET_UID))
	{
		mode |= S_ISUID;
	}
	if (modes.count(filemode::SET_GID))
	{
		mode |= S_ISGID;
	}
	if (modes.count(filemode::STICKY))
	{
		mode |= S_ISVTX;
	}
	return mode;
}

attributes::fileperms convert_file_perm(mode_t mode, mode_t r, mode_t w, mode_t x)
{
	using fileperm = attributes::fileperm;
	auto result    = attributes::fileperms{};
	if (mode & r)
	{
		result.insert(fileperm::READ);
	}
	if (mode & w)
	{
		result.insert(fileperm::WRITE);
	}
	if (mode & x)
	{
		result.insert(fileperm::EXEC);
	}
	return result;
}

mode_t convert_file_perm(const attributes::fileperms& perm, mode_t r, mode_t w, mode_t x)
{
	using fileperm = attributes::fileperm;
	auto mode      = mode_t{};
	if (perm.count(fileperm::READ))
	{
		mode |= r;
	}
	if (perm.count(fileperm::WRITE))
	{
		mode |= w;
	}
	if (perm.count(fileperm::EXEC))
	{
		mode |= x;
	}
	return mode;
}

} // namespace

bool attributes::is_dir() const
{
	return this->type == filetype::DIR;
}

bool attributes::is_reg() const
{
	return this->type == filetype::REG;
}

bool attributes::is_lnk() const
{
	return this->type == filetype::LNK;
}

mode_t attributes::get_mode() const
{
	return convert_file_perm(this->uperm, S_IRUSR, S_IWUSR, S_IXUSR) | //
	       convert_file_perm(this->gperm, S_IRGRP, S_IWGRP, S_IXGRP) | //
	       convert_file_perm(this->operm, S_IROTH, S_IWOTH, S_IXOTH) | //
	       convert_file_mode(this->mode) |                             //
	       convert_file_type(this->type);
}

void attributes::set_mode(mode_t mode)
{
	//fslog(debug, "mode={0:o}", mode);
	this->type  = convert_file_type(mode);
	this->mode  = convert_file_mode(mode);
	this->uperm = convert_file_perm(mode, S_IRUSR, S_IWUSR, S_IXUSR);
	this->gperm = convert_file_perm(mode, S_IRGRP, S_IWGRP, S_IXGRP);
	this->operm = convert_file_perm(mode, S_IROTH, S_IWOTH, S_IXOTH);
}

std::string attributes::mode_string() const
{
	char buf[10], *p = buf;
	switch (this->type)
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
	*p++ = this->uperm.count(fileperm::READ) ? 'r' : '-';
	*p++ = this->uperm.count(fileperm::WRITE) ? 'w' : '-';
	*p++ = this->uperm.count(fileperm::EXEC) ? 'x' : '-';
	*p++ = this->gperm.count(fileperm::READ) ? 'r' : '-';
	*p++ = this->gperm.count(fileperm::WRITE) ? 'w' : '-';
	*p++ = this->gperm.count(fileperm::EXEC) ? 'x' : '-';
	*p++ = this->operm.count(fileperm::READ) ? 'r' : '-';
	*p++ = this->operm.count(fileperm::WRITE) ? 'w' : '-';
	*p++ = this->operm.count(fileperm::EXEC) ? 'x' : '-';
	return std::string{ buf, sizeof(buf) };
}

std::optional<std::string> attributes::owner_or_uid() const
{
	if (this->owner)
	{
		return this->owner.value();
	}
	else if (this->uid)
	{
		return std::to_string(this->uid.value());
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<std::string> attributes::group_or_gid() const
{
	if (this->group)
	{
		return this->group.value();
	}
	else if (this->gid)
	{
		return std::to_string(this->gid.value());
	}
	else
	{
		return std::nullopt;
	}
}

} // namespace flexfs
