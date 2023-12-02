//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/api.h"
#include <cstdint>

namespace flexfs {

class FLEXFS_EXPORT i_file
{
public:
	virtual ~i_file() noexcept = default;

	virtual std::size_t read(void* buf, std::size_t count)        = 0;
	virtual std::size_t write(const void* buf, std::size_t count) = 0;
};

} // namespace flexfs
