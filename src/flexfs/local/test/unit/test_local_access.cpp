//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "local_fs_test_fixture.h"
#include "flexfs/local/local_access.h"
#include "flexfs/local/make_attributes.h"
#include "flexfs/core/noop_interruptor.h"
#include "flexfs/core/direntry.h"
#include "flexfs/core/i_file.h"
#include <boost/filesystem/operations.hpp>
#include <optional>
#include <gtest/gtest.h>

namespace flexfs {
namespace local {

class LocalAccessTests : public LocalFsTestFixture
{
};

TEST_F(LocalAccessTests, test_is_remote)
{
	auto a = access{ std::make_shared<noop_interruptor>() };
	EXPECT_FALSE(a.is_remote());
}

TEST_F(LocalAccessTests, test_ls_on_empty_dir)
{
	auto       a  = access{ std::make_shared<noop_interruptor>() };
	const auto ls = a.ls(this->work_dir());
	EXPECT_EQ(ls.size(), 0);
}

TEST_F(LocalAccessTests, test_ls_on_dir_with_1_file)
{
	const auto p = this->work_dir() / "file";
	this->touch(p);
	auto       a  = access{ std::make_shared<noop_interruptor>() };
	const auto ls = a.ls(this->work_dir());
	EXPECT_EQ(ls.size(), 1);
	const auto& e = ls.front();
	EXPECT_EQ(e.name, "file");
	EXPECT_EQ(e.attr, make_attributes(p, this->stat(p)));
}

TEST_F(LocalAccessTests, test_exists)
{
	const auto p = this->work_dir() / "file";
	auto       a = access{ std::make_shared<noop_interruptor>() };
	EXPECT_FALSE(a.exists(p));
	this->touch(p);
	EXPECT_TRUE(a.exists(p));
}

TEST_F(LocalAccessTests, test_try_stat)
{
	const auto p = this->work_dir() / "file";
	auto       a = access{ std::make_shared<noop_interruptor>() };
	EXPECT_FALSE(a.try_stat(p));
	this->touch(p);
	EXPECT_TRUE(a.try_stat(p));
	EXPECT_EQ(a.try_stat(p).value(), make_attributes(p, this->stat(p)));
}

TEST_F(LocalAccessTests, test_stat)
{
	const auto p = this->work_dir() / "file";
	auto       a = access{ std::make_shared<noop_interruptor>() };
	EXPECT_ANY_THROW(a.stat(p));
	this->touch(p);
	EXPECT_EQ(a.stat(p), make_attributes(p, this->stat(p)));
}

TEST_F(LocalAccessTests, test_lstat)
{
	const auto p = this->work_dir() / "file";
	auto       a = access{ std::make_shared<noop_interruptor>() };
	EXPECT_ANY_THROW(a.lstat(p));
	this->touch(p);
	EXPECT_EQ(a.lstat(p), make_attributes(p, this->lstat(p)));
}

TEST_F(LocalAccessTests, test_remove)
{
	const auto p = this->work_dir() / "file";
	auto       a = access{ std::make_shared<noop_interruptor>() };
	EXPECT_NO_THROW(a.remove(p));
	this->touch(p);
	EXPECT_NO_THROW(a.remove(p));
	EXPECT_FALSE(boost::filesystem::exists(p));
}

TEST_F(LocalAccessTests, test_mkdir_with_parents)
{
	{
		const auto p = this->work_dir() / "parent" / "child";
		auto       a = access{ std::make_shared<noop_interruptor>() };
		EXPECT_NO_THROW(a.mkdir(p, true));
		EXPECT_TRUE(boost::filesystem::is_directory(p));
	}
	{
		const auto p = this->work_dir() / "somedir";
		auto       a = access{ std::make_shared<noop_interruptor>() };
		EXPECT_NO_THROW(a.mkdir(p, true));
		EXPECT_TRUE(boost::filesystem::is_directory(p));
	}
}

TEST_F(LocalAccessTests, test_mkdir_without_parents)
{
	{
		const auto p = this->work_dir() / "parent" / "child";
		auto       a = access{ std::make_shared<noop_interruptor>() };
		EXPECT_ANY_THROW(a.mkdir(p, false));
		EXPECT_FALSE(boost::filesystem::is_directory(p));
	}
	{
		const auto p = this->work_dir() / "somedir";
		auto       a = access{ std::make_shared<noop_interruptor>() };
		EXPECT_NO_THROW(a.mkdir(p, true));
		EXPECT_TRUE(boost::filesystem::is_directory(p));
	}
}

TEST_F(LocalAccessTests, test_rename)
{
	const auto oldpath = this->work_dir() / "old";
	const auto newpath = this->work_dir() / "new";
	auto       a       = access{ std::make_shared<noop_interruptor>() };
	EXPECT_ANY_THROW(a.rename(oldpath, newpath));
	EXPECT_FALSE(boost::filesystem::exists(newpath));
	this->touch(oldpath);
	EXPECT_NO_THROW(a.rename(oldpath, newpath));
	EXPECT_FALSE(boost::filesystem::exists(oldpath));
	EXPECT_TRUE(boost::filesystem::exists(newpath));
}

TEST_F(LocalAccessTests, test_open)
{
	const auto p = this->work_dir() / "file";
	auto       a = access{ std::make_shared<noop_interruptor>() };
	EXPECT_ANY_THROW(a.open(p, O_RDONLY, 0));
	this->touch(p);
	EXPECT_NO_THROW(a.open(p, O_RDONLY, 0));
	EXPECT_NE(a.open(p, O_RDONLY, 0).get(), nullptr);
}

TEST_F(LocalAccessTests, test_create_watcher)
{
	const auto p = this->work_dir() / "dir";
	auto       a = access{ std::make_shared<noop_interruptor>() };
	EXPECT_ANY_THROW(a.create_watcher(p, 0));
	boost::filesystem::create_directory(p);
	EXPECT_NO_THROW(a.create_watcher(p, 0));
	EXPECT_NE(a.create_watcher(p, 0).get(), nullptr);
}

} // namespace local
} // namespace flexfs
