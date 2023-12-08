//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "local_fs_test_fixture.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/directory.hpp>

namespace flexfs {
namespace local {

LocalFsTestFixture::LocalFsTestFixture()
    : work_dir_{}
{
}

void LocalFsTestFixture::SetUp()
{
}

void LocalFsTestFixture::TearDown()
{
	if (this->work_dir_)
	{
		boost::filesystem::remove_all(this->work_dir_.value());
	}
}

const fspath& LocalFsTestFixture::work_dir() const
{
	if (!this->work_dir_)
	{
		this->work_dir_ =
		    boost::filesystem::unique_path(boost::filesystem::temp_directory_path() / "local_fs_test_suite-%%%%-%%%%-%%%%-%%%%");
		boost::filesystem::create_directory(this->work_dir_.value());
	}
	return this->work_dir_.value();
}

void LocalFsTestFixture::touch(const fspath& p) const
{
	auto os = boost::filesystem::ofstream{ p };
	os.close();
}

boost::filesystem::file_status LocalFsTestFixture::stat(const fspath& p) const
{
	return boost::filesystem::directory_entry{ p }.status();
}

boost::filesystem::file_status LocalFsTestFixture::lstat(const fspath& p) const
{
	return boost::filesystem::directory_entry{ p }.symlink_status();
}

} // namespace local
} // namespace flexfs
