//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "local_fs_test_fixture.h"
#include "flexfs/local/make_attributes.h"
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/operations.hpp>
#include <gtest/gtest.h>

namespace flexfs {
namespace local {

class MakeAttributesTests : public LocalFsTestFixture
{
};

TEST_F(MakeAttributesTests, test_make_filetype)
{
	EXPECT_EQ(make_filetype(boost::filesystem::status_error), attributes::filetype::UNKNOWN);
	EXPECT_EQ(make_filetype(boost::filesystem::file_not_found), attributes::filetype::UNKNOWN);
	EXPECT_EQ(make_filetype(boost::filesystem::regular_file), attributes::filetype::FILE);
	EXPECT_EQ(make_filetype(boost::filesystem::directory_file), attributes::filetype::DIR);
	EXPECT_EQ(make_filetype(boost::filesystem::symlink_file), attributes::filetype::LNK);
	EXPECT_EQ(make_filetype(boost::filesystem::block_file), attributes::filetype::BLOCK);
	EXPECT_EQ(make_filetype(boost::filesystem::character_file), attributes::filetype::CHAR);
	EXPECT_EQ(make_filetype(boost::filesystem::fifo_file), attributes::filetype::FIFO);
	EXPECT_EQ(make_filetype(boost::filesystem::socket_file), attributes::filetype::SOCK);
	EXPECT_EQ(make_filetype(boost::filesystem::reparse_file), attributes::filetype::UNKNOWN);
	EXPECT_EQ(make_filetype(boost::filesystem::type_unknown), attributes::filetype::UNKNOWN);
}

TEST_F(MakeAttributesTests, test_make_attributes)
{
	const auto p = this->work_dir() / "somefile";
	this->touch(p);
	const auto st = this->stat(p);
	const auto a  = make_attributes(p, st);
	EXPECT_EQ(a.type, attributes::filetype::FILE);
	EXPECT_TRUE(a.mode.empty());
	EXPECT_EQ(a.get_mode() & boost::filesystem::perms_mask, static_cast<mode_t>(st.permissions()) & boost::filesystem::perms_mask);
	EXPECT_TRUE(a.mtime);
	EXPECT_EQ(a.mtime.value(), std::chrono::system_clock::from_time_t(boost::filesystem::last_write_time(p)));
	EXPECT_TRUE(a.ctime);
	EXPECT_EQ(a.ctime.value(), std::chrono::system_clock::from_time_t(boost::filesystem::creation_time(p)));
	EXPECT_TRUE(a.size);
	EXPECT_EQ(a.size.value(), boost::filesystem::file_size(p));
}

} // namespace local
} // namespace flexfs
