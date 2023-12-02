//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "example_logger.h"

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <cstdio>

example_logger::example_logger()
    : mutex_{}
{
}

example_logger::~example_logger()
{
}

void example_logger::log_message(const std::chrono::system_clock::time_point& time,
                                 const boost::source_location& /*location*/,
                                 flexfs::log_level       level,
                                 const std::string_view& message)
{
	std::unique_lock lock{ this->mutex_ };

	fmt::print(stderr, "{:<8} [{}] {}\n", boost::describe::enum_to_string(level, "unknown"), time, message);
}
