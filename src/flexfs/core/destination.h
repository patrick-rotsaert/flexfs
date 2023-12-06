//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/api.h"
#include "flexfs/core/fspath.h"
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

	fspath path;                                            // The destination path.
	                                                        // --
	std::optional<time_expansion> expand_time_placeholders; // Expands time placeholders using the mtime of the source file.
	                                                        // See https://fmt.dev/latest/syntax.html#chrono-specs
	                                                        // The value controls if UTC or local time is used as input.
	                                                        // If not set (std::nullopt), then no time expansion takes place.
	                                                        // --
	bool create_parents;                                    // Recursively create parent directories of `path`?
	                                                        // --
	conflict_policy on_name_conflict;                       // How to handle an existing destination file.

	destination(const fspath&                        path,
	            const std::optional<time_expansion>& expand_time_placeholders,
	            bool                                 create_parents,
	            conflict_policy                      on_name_conflict);
};

} // namespace flexfs
