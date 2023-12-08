//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/api.h"
#include "flexfs/core/direntry.h"
#include "flexfs/core/fspath.h"
#include <boost/filesystem/directory.hpp>

namespace flexfs {
namespace local {

FLEXFS_LOCAL direntry make_direntry(const boost::filesystem::directory_entry& e);

}
} // namespace flexfs
