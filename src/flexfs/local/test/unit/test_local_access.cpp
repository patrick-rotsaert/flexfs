//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/local/local_access.h"
#include "flexfs/core/noop_interruptor.h"
#include "flexfs/core/direntry.h"
#include <boost/filesystem.hpp>
#include <optional>
#include <gtest/gtest.h>

namespace flexfs {
namespace local {

class LocalAccessTestSuite : public testing::Test
{
	mutable std::optional<fspath> work_dir_;

protected:
	LocalAccessTestSuite()
	    : work_dir_{}
	{
	}

	void SetUp() override
	{
	}

	void TearDown() override
	{
		if (this->work_dir_)
		{
			boost::filesystem::remove_all(this->work_dir_.value());
		}
	}

	const fspath& work_dir() const
	{
		if (!this->work_dir_)
		{
			this->work_dir_ =
			    boost::filesystem::unique_path(boost::filesystem::temp_directory_path() / "LocalAccessTestSuite-%%%%-%%%%-%%%%-%%%%");
			boost::filesystem::create_directory(this->work_dir_.value());
		}
		return this->work_dir_.value();
	}
};

TEST_F(LocalAccessTestSuite, test_is_remote)
{
	auto a = access{ std::make_shared<noop_interruptor>() };
	EXPECT_FALSE(a.is_remote());
}

TEST_F(LocalAccessTestSuite, test_ls_on_empty_dir)
{
	auto       a  = access{ std::make_shared<noop_interruptor>() };
	const auto ls = a.ls(this->work_dir());
	EXPECT_EQ(ls.size(), 0);
}

} // namespace local
} // namespace flexfs
