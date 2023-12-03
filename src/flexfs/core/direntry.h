//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/attributes.h"
#include "flexfs/core/fspath.h"
#include "flexfs/core/api.h"
#include <string>
#include <optional>

namespace flexfs {

class FLEXFS_EXPORT direntry final
{
public:
	std::string           name;
	attributes            attr;
	std::optional<fspath> symlink_target;

	direntry()                           = default;
	direntry(const direntry&)            = default;
	direntry(direntry&& src)             = default;
	direntry& operator=(const direntry&) = default;
	direntry& operator=(direntry&&)      = default;
};

} // namespace flexfs
