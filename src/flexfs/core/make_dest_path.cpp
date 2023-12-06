#include "flexfs/core/make_dest_path.h"
#include "flexfs/core/logging.h"
#include "flexfs/core/formatters.h"
#include "flexfs/core/exceptions.h"
#include "flexfs/core/attributes.h"
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <chrono>

namespace flexfs {

namespace {

bool path_ends_with_path_separator(const fspath& path)
{
	constexpr auto separators = std::string_view{ R"~(\/)~" };
	return separators.find(path.string().back()) != separators.npos;
}

} // namespace

fspath make_dest_path(std::shared_ptr<i_access> source_access,
                      const source&             source,
                      std::shared_ptr<i_access> dest_access,
                      const destination&        dest)
{
	auto new_path = dest.path;
	if (new_path.empty())
	{
		FLEXFS_THROW(invalid_argument_exception{} << error_mesg{ "destination path cannot be empty" });
	}

	if (dest.expand_time_placeholders)
	{
		const auto mtime = source_access->stat(source.current_path).mtime;
		if (mtime)
		{
			const auto tm = dest.expand_time_placeholders.value() == destination::time_expansion::LOCAL ? fmt::localtime(mtime.value())
			                                                                                            : fmt::gmtime(mtime.value());
			new_path      = fmt::format(fmt::runtime(new_path.string()), tm);
		}
		else
		{
			FLEXFS_THROW(
			    exception{ fmt::format("Unable to expand time placeholders in '{}' "
			                           "because its mtime is unavailable",
			                           source.current_path) });
		}
	}

	auto attr = dest_access->try_stat(new_path);
	if (attr)
	{
		auto&& resolve_name_conflict = [&]() {
			switch (dest.on_name_conflict)
			{
			case destination::conflict_policy::OVERWRITE:
				break;
			case destination::conflict_policy::AUTORENAME:
			{
				auto       i    = 0;
				const auto orig = new_path;
				do
				{
					new_path = orig.parent_path() /
					           fmt::format("{}~{}{}", orig.filename().stem().string(), ++i, orig.filename().extension().string());
				} while (dest_access->exists(new_path));
				break;
			}
			case destination::conflict_policy::FAIL:
				FLEXFS_THROW(system_exception{ std::error_code(EEXIST, std::system_category()) } << error_path{ new_path });
			}
		};

		// new_path exists.
		if (attr->is_dir())
		{
			// new_path is a directory.
			new_path /= source.orig_path.filename();
			attr = dest_access->try_stat(new_path);
			if (attr)
			{
				if (attr->is_dir())
				{
					FLEXFS_THROW(system_exception{ std::error_code(EISDIR, std::system_category()) } << error_path{ new_path });
				}
				else
				{
					// new_path exists and is not a dir
					resolve_name_conflict();
				}
			}
		}
		else if (path_ends_with_path_separator(new_path))
		{
			// new_path exists, it is not a directory and it ends with a path separator.
			FLEXFS_THROW(system_exception{ std::error_code(ENOTDIR, std::system_category()) } << error_path{ new_path });
		}
		else
		{
			// new_path exists, it is not a directory and does not end with a path separator.
			resolve_name_conflict();
		}
	}
	else
	{
		// new_path does not exist.
		if (path_ends_with_path_separator(new_path))
		{
			new_path /= source.orig_path.filename();
		}

		if (new_path.has_parent_path())
		{
			const auto parent = new_path.parent_path();
			if (dest.create_parents)
			{
				dest_access->mkdir(parent, true);
			}
			else
			{
				FLEXFS_THROW(system_exception{ std::error_code(ENOENT, std::system_category()) } << error_path{ parent });
			}
		}
	}
	return new_path;
}

} // namespace flexfs
