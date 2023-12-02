//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/api.h"
#include "flexfs/fspath.h"
#include <optional>

namespace flexfs {

class FLEXFS_EXPORT destination final
{
public:
	enum class conflict_policy
	{
		OVERWRITE,
		AUTORENAME,
		FAIL
	};

	enum class time_expansion
	{
		UTC,
		LOCAL
	};

	fspath                        path;
	std::optional<time_expansion> expand_time_placeholders; // see man strftime
	bool                          create_parents;
	conflict_policy               on_name_conflict;

	destination(const fspath&                        path,
	            const std::optional<time_expansion>& expand_time_placeholders,
	            bool                                 create_parents,
	            conflict_policy                      on_name_conflict);
};

} // namespace flexfs
