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
#include <functional>
#include <cstddef>

namespace flexfs {

// TODO: add documentation
FLEXFS_EXPORT void move_file(i_access& access, source& source, const destination& dest);

// TODO: add documentation
FLEXFS_EXPORT fspath copy_file(i_access&                                       source_access,
                               const source&                                   source,
                               i_access&                                       dest_access,
                               const destination&                              dest,
                               std::function<void(std::uint64_t bytes_copied)> on_progress = nullptr);

} // namespace flexfs
