//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/core/exceptions.h"
#include <boost/exception/get_error_info.hpp>
#include <gtest/gtest.h>

namespace flexfs {

TEST(ExceptionsTests, test_exception_default_ctor)
{
	auto e = exception{};
	EXPECT_NE(boost::get_error_info<error_uuid>(e), nullptr);
}

TEST(ExceptionsTests, test_exception_ctor_with_error_code)
{
	const auto ec = std::error_code{ ENOENT, std::system_category() };
	auto       e  = exception{ ec };
	EXPECT_NE(boost::get_error_info<error_uuid>(e), nullptr);
	EXPECT_EQ(*boost::get_error_info<error_code>(e), ec);
	EXPECT_EQ(*boost::get_error_info<error_mesg>(e), ec.message());
}

TEST(ExceptionsTests, test_exception_ctor_with_message)
{
	const auto m = std::string{ "some error" };
	auto       e = exception{ m };
	EXPECT_NE(boost::get_error_info<error_uuid>(e), nullptr);
	EXPECT_EQ(*boost::get_error_info<error_mesg>(e), m);
}

TEST(ExceptionsTests, test_exception_ctor_with_error_code_and_message)
{
	const auto ec = std::error_code{ ENOENT, std::system_category() };
	const auto m  = std::string{ "some error" };
	auto       e  = exception{ ec, m };
	EXPECT_NE(boost::get_error_info<error_uuid>(e), nullptr);
	EXPECT_EQ(*boost::get_error_info<error_code>(e), ec);
	EXPECT_EQ(*boost::get_error_info<error_mesg>(e), m + ": " + ec.message());
}

TEST(ExceptionsTests, test_exception_what_with_message)
{
	const auto e = exception{ "some error" };
	EXPECT_STREQ(e.what(), "some error");
}

TEST(ExceptionsTests, test_exception_what_without_message)
{
	const auto e = exception{};
	EXPECT_STREQ(e.what(), "std::exception");
}

TEST(ExceptionsTests, test_system_exception_default_ctor)
{
	auto e = system_exception{};
	EXPECT_NE(boost::get_error_info<error_uuid>(e), nullptr);
	EXPECT_NE(boost::get_error_info<error_code>(e), nullptr);
}

TEST(ExceptionsTests, test_system_exception_ctor_with_error_code)
{
	const auto ec = std::error_code{ ENOENT, std::system_category() };
	auto       e  = system_exception{ ec };
	EXPECT_NE(boost::get_error_info<error_uuid>(e), nullptr);
	EXPECT_EQ(*boost::get_error_info<error_code>(e), ec);
	EXPECT_EQ(*boost::get_error_info<error_mesg>(e), ec.message());
}

TEST(ExceptionsTests, test_system_exception_ctor_with_message)
{
	const auto m = std::string{ "some error" };
	auto       e = system_exception{ m };
	EXPECT_NE(boost::get_error_info<error_uuid>(e), nullptr);
	EXPECT_NE(boost::get_error_info<error_code>(e), nullptr);
	const auto& ec = *boost::get_error_info<error_code>(e);
	EXPECT_EQ(*boost::get_error_info<error_mesg>(e), m + ": " + ec.message());
}

TEST(ExceptionsTests, test_system_exception_ctor_with_error_code_and_message)
{
	const auto ec = std::error_code{ ENOENT, std::system_category() };
	const auto m  = std::string{ "some error" };
	auto       e  = system_exception{ ec, m };
	EXPECT_NE(boost::get_error_info<error_uuid>(e), nullptr);
	EXPECT_EQ(*boost::get_error_info<error_code>(e), ec);
	EXPECT_EQ(*boost::get_error_info<error_mesg>(e), m + ": " + ec.message());
}

} // namespace flexfs
