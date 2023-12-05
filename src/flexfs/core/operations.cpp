//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/core/operations.h"
#include "flexfs/core/logging.h"
#include "flexfs/core/formatters.h"
#include "flexfs/core/exceptions.h"
#include "flexfs/core/attributes.h"
#include "flexfs/core/i_file.h"
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <chrono>
#include <cassert>

namespace flexfs {

namespace {

fspath make_dest_path(std::shared_ptr<i_access> access, const source& source, const destination& dest)
{
	auto new_path = dest.path;
	if (new_path.empty())
	{
		FLEXFS_THROW(should_not_happen_exception{} << error_mesg{ "destination path cannot be empty" });
	}
	if (dest.expand_time_placeholders)
	{
		const auto mtime = access->stat(source.current_path).mtime;
		if (mtime)
		{
			const auto tm = dest.expand_time_placeholders.value() == destination::time_expansion::LOCAL ? fmt::localtime(mtime.value())
			                                                                                            : fmt::gmtime(mtime.value());
			new_path      = fmt::format(fmt::runtime(new_path.string()), tm);
		}
		else
		{
			fslog(warn, "Cannot get mtime of {}", source.current_path);
		}
	}
	auto attr = access->try_stat(new_path);
	if (attr)
	{
		// newpath exists
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
				} while (access->exists(new_path));
				break;
			}
			case destination::conflict_policy::FAIL:
				FLEXFS_THROW(system_exception{ std::error_code(EEXIST, std::system_category()) } << error_path{ new_path });
			}
		};
		if (attr->is_dir())
		{
			new_path /= source.orig_path.filename();
			attr = access->try_stat(new_path);
			if (attr)
			{
				if (attr->is_dir())
				{
					FLEXFS_THROW(system_exception{ std::error_code(EISDIR, std::system_category()) } << error_path{ new_path });
				}
				else
				{
					// newpath exists and is not a dir
					resolve_name_conflict();
				}
			}
		}
		else if (new_path.string().back() == '/')
		{
			FLEXFS_THROW(system_exception{ std::error_code(ENOTDIR, std::system_category()) } << error_path{ new_path });
		}
		else
		{
			resolve_name_conflict();
		}
	}
	else
	{
		// newpath does not exist
		if (new_path.string().back() == '/')
		{
			new_path /= source.orig_path.filename();
		}
		if (new_path.has_parent_path())
		{
			const auto parent = new_path.parent_path();
			if (!access->exists(parent))
			{
				if (dest.create_parents)
				{
					access->mkdir(parent, true);
				}
				else
				{
					FLEXFS_THROW(system_exception{ std::error_code(ENOENT, std::system_category()) } << error_path{ parent });
				}
			}
		}
	}
	return new_path;
}

} // namespace

void move_file(std::shared_ptr<i_access> access, source& source, const destination& dest)
{
	const auto new_path = make_dest_path(access, source, dest);
	access->rename(source.current_path, new_path);
	source.current_path = new_path;
}

fspath copy_file(std::shared_ptr<i_access>                       source_access,
                 const source&                                   source,
                 std::shared_ptr<i_access>                       dest_access,
                 const destination&                              dest,
                 std::function<void(std::uint64_t bytes_copied)> on_progress)
{
	auto in = source_access->open(source.current_path, O_RDONLY | O_BINARY, 0);

	const auto dest_path = make_dest_path(dest_access, source, dest);

	auto out = dest_access->open(dest_path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, source_access->stat(source.current_path).get_mode());

	std::array<char, 65536u> buf{};
	std::uint64_t            bytes_copied{};

	for (;;)
	{
		const auto count = in->read(buf.data(), buf.size());
		assert(count <= buf.size());
		if (count == 0)
		{
			break;
		}
		else
		{
			auto ptr = buf.data();
			for (auto writecount = count; writecount;)
			{
				const auto written = out->write(ptr, writecount);
				assert(written <= writecount);
				writecount -= written;
				ptr += written;
				if (on_progress)
				{
					bytes_copied += written;
					on_progress(bytes_copied);
				}
			}
		}
	}

	return dest_path;
}

} // namespace flexfs
