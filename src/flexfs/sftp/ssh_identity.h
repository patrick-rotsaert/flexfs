//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/api.h"
#include <string>

namespace flexfs {
namespace sftp {

class FLEXFS_EXPORT ssh_identity
{
public:
	std::string name;
	std::string pkey;
};

} // namespace sftp
} // namespace flexfs
