//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/api.h"
#include "flexfs/core/fspath.h"

namespace flexfs {

class FLEXFS_EXPORT source
{
public:
	fspath orig_path;
	fspath current_path; // Differs from `orig_path` after moving the file (see operations.h)

	explicit source(const fspath& path);
};

} // namespace flexfs
