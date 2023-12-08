//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/api.h"
#include "flexfs/core/attributes.h"
#include "flexfs/core/fspath.h"
#include <boost/filesystem/file_status.hpp>

namespace flexfs {
namespace local {

FLEXFS_LOCAL attributes::filetype make_filetype(boost::filesystem::file_type type);
FLEXFS_LOCAL attributes           make_attributes(const fspath& path, const boost::filesystem::file_status& st);

} // namespace local
} // namespace flexfs
