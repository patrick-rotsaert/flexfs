//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/i_logger.h"

#include <mutex>

class example_logger final : public flexfs::i_logger
{
	std::mutex mutex_;

public:
	example_logger();
	~example_logger() override;

	void log_message(const std::chrono::system_clock::time_point& time,
	                 const boost::source_location&                location,
	                 flexfs::log_level                            level,
	                 const std::string_view&                      message) override;
};
