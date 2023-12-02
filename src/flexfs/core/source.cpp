//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/core/source.h"

namespace flexfs {

source::source(const fspath& path)
    : orig_path{ path }
    , current_path{ path }
{
}

} // namespace flexfs
