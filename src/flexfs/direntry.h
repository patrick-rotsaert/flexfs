//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/attributes.h"
#include "flexfs/api.h"
#include <string>
#include <optional>

namespace flexfs {

class FLEXFS_EXPORT direntry final
{
public:
	std::string name;
	attributes  attr;
	// FIXME: should be direntry
	std::optional<attributes> link; // attributes of the dereferenced link, only set if attr.type == LINK and file is not a dead link

	direntry()                           = default;
	direntry(const direntry&)            = default;
	direntry(direntry&& src)             = default;
	direntry& operator=(const direntry&) = default;
	direntry& operator=(direntry&&)      = default;
};

} // namespace flexfs
