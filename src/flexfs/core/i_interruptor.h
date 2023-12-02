//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/api.h"
#include "flexfs/core/exceptions.h"
#include <chrono>

namespace flexfs {

struct FLEXFS_EXPORT interrupted_exception : public exception
{
};

class FLEXFS_EXPORT i_interruptor
{
public:
	virtual ~i_interruptor() noexcept;

	virtual void interrupt()                                               = 0;
	virtual bool is_interrupted()                                          = 0;
	virtual bool wait_for_interruption(std::chrono::milliseconds duration) = 0; // returns true if interrupted, false on timeout

	void throw_if_interrupted(); // throws interrupted_exception if interrupted()
};

} // namespace flexfs
