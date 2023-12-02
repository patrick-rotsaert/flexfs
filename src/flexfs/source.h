//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/api.h"
#include "flexfs/fspath.h"

namespace flexfs {

class FLEXFS_EXPORT source
{
public:
	fspath orig_path, current_path;

	explicit source(const fspath& path);
};

} // namespace flexfs
