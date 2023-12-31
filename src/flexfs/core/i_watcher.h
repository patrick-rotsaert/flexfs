//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/api.h"
#include "flexfs/core/direntry.h"
#include <vector>
#include <memory>

namespace flexfs {

class FLEXFS_EXPORT i_watcher
{
public:
	virtual ~i_watcher() noexcept;

	virtual std::vector<direntry> watch() = 0;
};

} // namespace flexfs
