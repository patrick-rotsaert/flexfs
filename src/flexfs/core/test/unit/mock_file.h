//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/i_file.h"
#include <gmock/gmock.h>

namespace flexfs {

class FLEXFS_EXPORT mock_file : public i_file
{
public:
	explicit mock_file();
	~mock_file() override;

	MOCK_METHOD(std::size_t, read, (void* buf, std::size_t count), (override));
	MOCK_METHOD(std::size_t, write, (const void* buf, std::size_t count), (override));
};

// This regex search and replace patterns can help to convert declarations into MOCK_METHOD macro calls
// Search pattern: ^(.+)\s([a-zA-Z_][a-zA-Z0-9_]*)\s*(\([^)]*\))\s*override\s*;\s*$
// Replace pattern: MOCK_METHOD(\1,\2,\3,(override));

using nice_mock_file   = testing::NiceMock<mock_file>;
using naggy_mock_file  = testing::NaggyMock<mock_file>;
using strict_mock_file = testing::StrictMock<mock_file>;

} // namespace flexfs
