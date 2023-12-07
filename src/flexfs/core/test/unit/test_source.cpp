//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/core/source.h"
#include <gtest/gtest.h>

namespace flexfs {

TEST(SourceTests, test_ctor)
{
	const auto p   = fspath{ "foo" };
	const auto src = source{ p };
	EXPECT_EQ(src.orig_path, p);
	EXPECT_EQ(src.orig_path, src.current_path);
}

} // namespace flexfs
