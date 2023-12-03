//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/api.h"
#include "flexfs/core/i_logger.h"
#include "flexfs/core/config.h"
#include <fmt/format.h>
#include <boost/assert/source_location.hpp>
#include <memory>
#include <chrono>

namespace flexfs {

class FLEXFS_EXPORT logging
{
public:
	static std::unique_ptr<i_logger> logger;

	// Not thread safe
	static void set_logger(std::unique_ptr<i_logger>&& logger);
};

} // namespace flexfs

// Macro for logging using a fmtlib format string
#undef fslog
#define fslog(lvl, ...)                                                                                                                    \
	do                                                                                                                                     \
	{                                                                                                                                      \
		auto& logger = ::flexfs::logging::logger;                                                                                          \
		if (logger)                                                                                                                        \
		{                                                                                                                                  \
			logger->log_message(                                                                                                           \
			    std::chrono::system_clock::now(), BOOST_CURRENT_LOCATION, ::flexfs::log_level::lvl, fmt::format(__VA_ARGS__));             \
		}                                                                                                                                  \
	} while (false)

// Macros for eliding logging code at compile time
#undef FLEXFS_MIN_LOG
#define FLEXFS_MIN_LOG(minlvl, lvl, ...)                                                                                                   \
	do                                                                                                                                     \
	{                                                                                                                                      \
		if constexpr (::flexfs::log_level::lvl >= ::flexfs::log_level::minlvl)                                                             \
		{                                                                                                                                  \
			slog(lvl, __VA_ARGS__);                                                                                                        \
		}                                                                                                                                  \
	} while (false)

#undef FLEXFS_LOG
#define FLEXFS_LOG(lvl, ...) FLEXFS_MIN_LOG(FLEXFS_LOGGING_LEVEL, lvl, __VA_ARGS__)
