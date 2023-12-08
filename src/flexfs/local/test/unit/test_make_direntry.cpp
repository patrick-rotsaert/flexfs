//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "local_fs_test_fixture.h"
#include "flexfs/local/make_direntry.h"
#include <boost/filesystem/operations.hpp>
#include <gtest/gtest.h>

namespace flexfs {
namespace local {

class MakeDirentryTests : public LocalFsTestFixture
{
};

TEST_F(MakeDirentryTests, test_make_direntry_from_file)
{
	const auto p = this->work_dir() / "somefile";
	this->touch(p);
	const auto e = make_direntry(boost::filesystem::directory_entry{ p });
	EXPECT_EQ(e.name, p.filename().string());
	EXPECT_TRUE(e.attr.is_reg());
	EXPECT_FALSE(e.symlink_target.has_value());
}

TEST_F(MakeDirentryTests, test_make_direntry_from_symlink)
{
	const auto p   = this->work_dir() / "somefile";
	const auto lnk = this->work_dir() / "somelink";
	this->touch(p);
	boost::filesystem::create_symlink(p, lnk);
	const auto e = make_direntry(boost::filesystem::directory_entry{ lnk });
	EXPECT_EQ(e.name, lnk.filename().string());
	EXPECT_TRUE(e.attr.is_lnk());
	EXPECT_TRUE(e.symlink_target.has_value());
	EXPECT_EQ(e.symlink_target.value(), p);
}

TEST_F(MakeDirentryTests, test_make_direntry_from_dead_symlink)
{
	const auto p   = this->work_dir() / "somefile";
	const auto lnk = this->work_dir() / "somelink";
	boost::filesystem::create_symlink(p, lnk);
	const auto e = make_direntry(boost::filesystem::directory_entry{ lnk });
	EXPECT_EQ(e.name, lnk.filename().string());
	EXPECT_TRUE(e.attr.is_lnk());
	EXPECT_TRUE(e.symlink_target.has_value());
	EXPECT_EQ(e.symlink_target.value(), p);
}

} // namespace local
} // namespace flexfs
