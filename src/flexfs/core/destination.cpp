//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/core/destination.h"

namespace flexfs {

destination::destination(const fspath&                        path,
                         const std::optional<time_expansion>& expand_time_placeholders,
                         bool                                 create_parents,
                         conflict_policy                      on_name_conflict)
    : path{ path }
    , expand_time_placeholders{ expand_time_placeholders }
    , create_parents{ create_parents }
    , on_name_conflict{ on_name_conflict }
{
}

} // namespace flexfs
