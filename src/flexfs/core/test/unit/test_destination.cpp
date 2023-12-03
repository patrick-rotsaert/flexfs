//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>
#include "flexfs/core/destination.h"
#include <cmath>
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

TEST(DestinationTests, test_ctor)
{
	{
		auto x = destination{ "/foo/bar", std::nullopt, false, destination::conflict_policy::OVERWRITE };
		EXPECT_EQ(x.path, "/foo/bar");
		EXPECT_EQ(x.expand_time_placeholders, std::nullopt);
		EXPECT_FALSE(x.create_parents);
		EXPECT_EQ(x.on_name_conflict, destination::conflict_policy::OVERWRITE);
	}
	{
		auto x = destination{ "/foo/bar", destination::time_expansion::UTC, true, destination::conflict_policy::FAIL };
		EXPECT_EQ(x.path, "/foo/bar");
		EXPECT_EQ(x.expand_time_placeholders, destination::time_expansion::UTC);
		EXPECT_TRUE(x.create_parents);
		EXPECT_EQ(x.on_name_conflict, destination::conflict_policy::FAIL);
	}
}

} // namespace flexfs
