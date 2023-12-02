//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "example_logger.h"
#include "flexfs/logging.h"

int main()
{
	flexfs::logging::set_logger(std::make_unique<example_logger>());
	fslog(debug, "Hello {}!", "world");
}
