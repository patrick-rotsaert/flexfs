//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/core/attributes.h"
#include <gtest/gtest.h>
#include <set>

namespace flexfs {

namespace {

template<typename T, typename... Args>
std::set<T> make_set(T first, Args... args)
{
	auto result = std::set<T>{};
	result.insert(first);
	if constexpr (sizeof...(args) > 0)
	{
		result.merge(make_set(args...));
	}
	return result;
}

} // namespace

TEST(AttributesTests, test_is_dir)
{
	auto a = attributes{};
	a.type = attributes::filetype::DIR;
	EXPECT_TRUE(a.is_dir());
	a.type = attributes::filetype::REG;
	EXPECT_FALSE(a.is_dir());
}

TEST(AttributesTests, test_is_reg)
{
	auto a = attributes{};
	a.type = attributes::filetype::REG;
	EXPECT_TRUE(a.is_reg());
	a.type = attributes::filetype::DIR;
	EXPECT_FALSE(a.is_reg());
}

TEST(AttributesTests, test_is_lnk)
{
	auto a = attributes{};
	a.type = attributes::filetype::LNK;
	EXPECT_TRUE(a.is_lnk());
	a.type = attributes::filetype::REG;
	EXPECT_FALSE(a.is_lnk());
}

TEST(AttributesTests, test_get_mode)
{
	{
		auto a  = attributes{};
		a.mode  = { attributes::filemode::SET_GID, attributes::filemode::STICKY };
		a.type  = attributes::filetype::BLOCK;
		a.uperm = { attributes::fileperm::READ };
		a.gperm = { attributes::fileperm::READ, attributes::fileperm::EXEC };
		a.operm = { attributes::fileperm::WRITE, attributes::fileperm::EXEC };
		EXPECT_EQ(a.get_mode(),           //
		          S_ISGID | S_ISVTX |     //
		              S_IFBLK |           //
		              S_IRUSR |           //
		              S_IRGRP | S_IXGRP | //
		              S_IWOTH | S_IXOTH | //
		              0);
	}

	{
		auto a  = attributes{};
		a.mode  = { attributes::filemode::SET_UID };
		a.type  = attributes::filetype::CHAR;
		a.uperm = { attributes::fileperm::READ, attributes::fileperm::WRITE };
		a.gperm = { attributes::fileperm::READ, attributes::fileperm::WRITE, attributes::fileperm::EXEC };
		a.operm = { attributes::fileperm::READ, attributes::fileperm::WRITE, attributes::fileperm::EXEC };
		EXPECT_EQ(a.get_mode(),                     //
		          S_ISUID |                         //
		              S_IFCHR |                     //
		              S_IRUSR | S_IWUSR |           //
		              S_IRGRP | S_IWGRP | S_IXGRP | //
		              S_IROTH | S_IWOTH | S_IXOTH | //
		              0);
	}

	{
		auto a  = attributes{};
		a.mode  = { attributes::filemode::SET_GID };
		a.type  = attributes::filetype::DIR;
		a.uperm = { attributes::fileperm::READ, attributes::fileperm::WRITE, attributes::fileperm::EXEC };
		a.gperm = { attributes::fileperm::WRITE, attributes::fileperm::EXEC };
		a.operm = { attributes::fileperm::EXEC };
		EXPECT_EQ(a.get_mode(),                     //
		          S_ISGID |                         //
		              S_IFDIR |                     //
		              S_IRUSR | S_IWUSR | S_IXUSR | //
		              S_IWGRP | S_IXGRP |           //
		              S_IXOTH |                     //
		              0);
	}

	{
		auto a  = attributes{};
		a.mode  = { attributes::filemode::STICKY };
		a.type  = attributes::filetype::FIFO;
		a.uperm = { attributes::fileperm::WRITE };
		a.gperm = { attributes::fileperm::EXEC };
		a.operm = { attributes::fileperm::READ };
		EXPECT_EQ(a.get_mode(), //
		          S_ISVTX |     //
		              S_IFIFO | //
		              S_IWUSR | //
		              S_IXGRP | //
		              S_IROTH | //
		              0);
	}

	{
		auto a = attributes{};
		a.type = attributes::filetype::LINK;
		EXPECT_EQ(a.get_mode(), S_IFLNK);

		a.type = attributes::filetype::FILE;
		EXPECT_EQ(a.get_mode(), S_IFREG);

		a.type = attributes::filetype::SOCK;
		EXPECT_EQ(a.get_mode(), S_IFSOCK);

		a.type = attributes::filetype::SPECIAL;
		EXPECT_EQ(a.get_mode(), 0);

		a.type = attributes::filetype::UNKNOWN;
		EXPECT_EQ(a.get_mode(), 0);
	}
}

TEST(AttributesTests, test_set_mode)
{
	{
		auto a = attributes{};
		a.set_mode(             //
		    S_ISGID | S_ISVTX | //
		    S_IFBLK |           //
		    S_IRUSR |           //
		    S_IRGRP | S_IXGRP | //
		    S_IWOTH | S_IXOTH | //
		    0);
		EXPECT_EQ(a.type, attributes::filetype::BLOCK);
		EXPECT_EQ(a.mode, make_set(attributes::filemode::SET_GID, attributes::filemode::STICKY));
		EXPECT_EQ(a.uperm, make_set(attributes::fileperm::READ));
		EXPECT_EQ(a.gperm, make_set(attributes::fileperm::READ, attributes::fileperm::EXEC));
		EXPECT_EQ(a.operm, make_set(attributes::fileperm::WRITE, attributes::fileperm::EXEC));
	}

	{
		auto a = attributes{};
		a.set_mode(                       //
		    S_ISUID |                     //
		    S_IFCHR |                     //
		    S_IRUSR | S_IWUSR |           //
		    S_IRGRP | S_IWGRP | S_IXGRP | //
		    S_IROTH | S_IWOTH | S_IXOTH | //
		    0);
		EXPECT_EQ(a.type, attributes::filetype::CHAR);
		EXPECT_EQ(a.mode, make_set(attributes::filemode::SET_UID));
		EXPECT_EQ(a.uperm, make_set(attributes::fileperm::READ, attributes::fileperm::WRITE));
		EXPECT_EQ(a.gperm, make_set(attributes::fileperm::READ, attributes::fileperm::WRITE, attributes::fileperm::EXEC));
		EXPECT_EQ(a.operm, make_set(attributes::fileperm::READ, attributes::fileperm::WRITE, attributes::fileperm::EXEC));
	}

	{
		auto a = attributes{};
		a.set_mode(                       //
		    S_ISGID |                     //
		    S_IFDIR |                     //
		    S_IRUSR | S_IWUSR | S_IXUSR | //
		    S_IWGRP | S_IXGRP |           //
		    S_IXOTH |                     //
		    0);
		EXPECT_EQ(a.type, attributes::filetype::DIR);
		EXPECT_EQ(a.mode, make_set(attributes::filemode::SET_GID));
		EXPECT_EQ(a.uperm, make_set(attributes::fileperm::READ, attributes::fileperm::WRITE, attributes::fileperm::EXEC));
		EXPECT_EQ(a.gperm, make_set(attributes::fileperm::WRITE, attributes::fileperm::EXEC));
		EXPECT_EQ(a.operm, make_set(attributes::fileperm::EXEC));
	}

	{
		auto a = attributes{};
		a.set_mode(   //
		    S_ISVTX | //
		    S_IFIFO | //
		    S_IWUSR | //
		    S_IXGRP | //
		    S_IROTH | //
		    0);
		EXPECT_EQ(a.type, attributes::filetype::FIFO);
		EXPECT_EQ(a.mode, make_set(attributes::filemode::STICKY));
		EXPECT_EQ(a.uperm, make_set(attributes::fileperm::WRITE));
		EXPECT_EQ(a.gperm, make_set(attributes::fileperm::EXEC));
		EXPECT_EQ(a.operm, make_set(attributes::fileperm::READ));
	}

	{
		auto a = attributes{};
		a.set_mode(S_IFLNK);
		EXPECT_EQ(a.type, attributes::filetype::LINK);

		a.set_mode(S_IFREG);
		EXPECT_EQ(a.type, attributes::filetype::FILE);

		a.set_mode(S_IFSOCK);
		EXPECT_EQ(a.type, attributes::filetype::SOCK);

		a.set_mode(0);
		EXPECT_EQ(a.type, attributes::filetype::UNKNOWN);
	}
}

TEST(AttributesTests, test_mode_string)
{
	auto a = attributes{};

	a.type = attributes::filetype::BLOCK;
	EXPECT_EQ(a.mode_string(), "b---------");

	a.type = attributes::filetype::CHAR;
	EXPECT_EQ(a.mode_string(), "c---------");

	a.type = attributes::filetype::DIR;
	EXPECT_EQ(a.mode_string(), "d---------");

	a.type = attributes::filetype::FIFO;
	EXPECT_EQ(a.mode_string(), "p---------");

	a.type = attributes::filetype::LINK;
	EXPECT_EQ(a.mode_string(), "l---------");

	a.type = attributes::filetype::FILE;
	EXPECT_EQ(a.mode_string(), "----------");

	a.type = attributes::filetype::SOCK;
	EXPECT_EQ(a.mode_string(), "s---------");

	a.type = attributes::filetype::SPECIAL;
	EXPECT_EQ(a.mode_string(), "S---------");

	a.type = attributes::filetype::UNKNOWN;
	EXPECT_EQ(a.mode_string(), "?---------");

	a.uperm = { attributes::fileperm::READ };
	EXPECT_EQ(a.mode_string(), "?r--------");

	a.uperm = { attributes::fileperm::READ, attributes::fileperm::WRITE };
	EXPECT_EQ(a.mode_string(), "?rw-------");

	a.uperm = { attributes::fileperm::READ, attributes::fileperm::WRITE, attributes::fileperm::EXEC };
	EXPECT_EQ(a.mode_string(), "?rwx------");

	a.gperm = { attributes::fileperm::READ };
	EXPECT_EQ(a.mode_string(), "?rwxr-----");

	a.gperm = { attributes::fileperm::READ, attributes::fileperm::WRITE };
	EXPECT_EQ(a.mode_string(), "?rwxrw----");

	a.gperm = { attributes::fileperm::READ, attributes::fileperm::WRITE, attributes::fileperm::EXEC };
	EXPECT_EQ(a.mode_string(), "?rwxrwx---");

	a.operm = { attributes::fileperm::READ };
	EXPECT_EQ(a.mode_string(), "?rwxrwxr--");

	a.operm = { attributes::fileperm::READ, attributes::fileperm::WRITE };
	EXPECT_EQ(a.mode_string(), "?rwxrwxrw-");

	a.operm = { attributes::fileperm::READ, attributes::fileperm::WRITE, attributes::fileperm::EXEC };
	EXPECT_EQ(a.mode_string(), "?rwxrwxrwx");
}

TEST(AttributesTests, test_owner_or_uid)
{
	auto a = attributes{};

	a.owner = "abc";
	a.uid   = 1234;
	EXPECT_EQ(a.owner_or_uid(), "abc");

	a.owner = std::nullopt;
	EXPECT_EQ(a.owner_or_uid(), "1234");

	a.uid = std::nullopt;
	EXPECT_EQ(a.owner_or_uid(), std::nullopt);
}

TEST(AttributesTests, test_group_or_gid)
{
	auto a = attributes{};

	a.group = "abc";
	a.gid   = 1234;
	EXPECT_EQ(a.group_or_gid(), "abc");

	a.group = std::nullopt;
	EXPECT_EQ(a.group_or_gid(), "1234");

	a.gid = std::nullopt;
	EXPECT_EQ(a.group_or_gid(), std::nullopt);
}

} // namespace flexfs
