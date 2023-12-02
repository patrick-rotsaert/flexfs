//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/core/version.h"

#include <cassert>

namespace flexfs {

int version::number()
{
	return FLEXFS_VERSION_NUMBER;
}

int version::major()
{
	return FLEXFS_VERSION_MAJOR;
}

int version::minor()
{
	return FLEXFS_VERSION_MINOR;
}

int version::patch()
{
	return FLEXFS_VERSION_PATCH;
}

void version::check()
{
	assert(FLEXFS_VERSION_NUMBER == number());
}

} // namespace flexfs
