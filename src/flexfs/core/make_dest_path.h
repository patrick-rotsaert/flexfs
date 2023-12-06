//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/api.h"
#include "flexfs/core/i_access.h"
#include "flexfs/core/fspath.h"
#include "flexfs/core/source.h"
#include "flexfs/core/destination.h"

namespace flexfs {

FLEXFS_LOCAL fspath make_dest_path(std::shared_ptr<i_access> source_access,
                                   const source&             source,
                                   std::shared_ptr<i_access> dest_access,
                                   const destination&        dest);

} // namespace flexfs
