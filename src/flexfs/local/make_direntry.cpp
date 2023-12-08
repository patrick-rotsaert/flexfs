//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/local/make_direntry.h"
#include "flexfs/local/make_attributes.h"
#include "flexfs/core/logging.h"
#include "flexfs/core/formatters.h"
#include "flexfs/core/exceptions.h"
#include <boost/filesystem/operations.hpp>

namespace flexfs {
namespace local {

direntry make_direntry(const boost::filesystem::directory_entry& e)
{
	const auto& path = e.path();
	const auto  st   = e.symlink_status();

	auto result = direntry{};

	result.name = path.filename().string();
	result.attr = make_attributes(path, st);

	if (st.type() == boost::filesystem::symlink_file)
	{
		fslog(trace, "read_symlink path={}", path);
		auto ec               = boost::system::error_code{};
		result.symlink_target = boost::filesystem::read_symlink(path, ec);
		if (ec)
		{
			FLEXFS_THROW(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "read_symlink" });
		}
	}

	return result;
}

} // namespace local
} // namespace flexfs
