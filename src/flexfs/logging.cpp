//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/logging.h"
#include "flexfs/config.h"
#ifdef FLEXFS_USE_SPDLOG
#include "flexfs/spdlog_logger.h"
#endif

namespace flexfs {

#ifdef FLEXFS_USE_SPDLOG
std::unique_ptr<i_logger> logging::logger = std::make_unique<spdlog_logger>();
#else
std::unique_ptr<i_logger> logging::logger{};
#endif

void logging::set_logger(std::unique_ptr<i_logger>&& logger)
{
	logging::logger = std::move(logger);
}

} // namespace flexfs
