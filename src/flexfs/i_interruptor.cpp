//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/i_interruptor.h"

namespace flexfs {

i_interruptor::~i_interruptor() noexcept
{
}

void i_interruptor::throw_if_interrupted()
{
	if (is_interrupted())
	{
		BOOST_THROW_EXCEPTION(interrupted_exception{});
	}
}

} // namespace flexfs
