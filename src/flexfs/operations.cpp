//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/operations.h"
#include "flexfs/logging.h"
#include "flexfs/formatters.h"
#include "flexfs/exceptions.h"
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/scoped_array.hpp>
#include <fmt/format.h>
#include <ctime>
#include <cassert>
#include <array>

namespace flexfs {

namespace {

fspath make_dest_path(std::shared_ptr<i_access> access, const source& source, const destination& dest)
{
	auto newpath = dest.path;
	if (newpath.empty())
	{
		FLEXFS_THROW(should_not_happen_exception{} << error_mesg{ "destination path cannot be empty" });
	}
	if (dest.expand_time_placeholders)
	{
		const auto mtime = access->stat(source.current_path).mtime;
		if (mtime)
		{
			using adjustor     = boost::date_time::c_local_adjustor<boost::posix_time::ptime>;
			const auto tm      = boost::posix_time::to_tm(dest.expand_time_placeholders.value() == destination::time_expansion::LOCAL
                                                         ? adjustor::utc_to_local(mtime.value())
                                                         : mtime.value());
			auto       bufsize = newpath.string().length();
			for (;;)
			{
				boost::scoped_array<char> buf(new char[bufsize]);
				const auto                length = strftime(buf.get(), bufsize, newpath.string().c_str(), &tm);
				if (length == 0)
				{
					bufsize *= 2;
				}
				else
				{
					newpath = std::string(buf.get(), length);
					break;
				}
			}
		}
		else
		{
			fslog(warn, "Cannot get mtime of {}", source.current_path);
		}
	}
	auto attr = access->try_stat(newpath);
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
				const auto orig = newpath;
				do
				{
					newpath = orig.parent_path() /
					          fmt::format("{}~{}{}", orig.filename().stem().string(), ++i, orig.filename().extension().string());
				} while (access->exists(newpath));
				break;
			}
			case destination::conflict_policy::FAIL:
				FLEXFS_THROW(system_exception{ std::error_code(EEXIST, std::system_category()) } << error_path{ newpath });
			}
		};
		if (attr->is_dir())
		{
			newpath /= source.orig_path.filename();
			attr = access->try_stat(newpath);
			if (attr)
			{
				if (attr->is_dir())
				{
					FLEXFS_THROW(system_exception{ std::error_code(EISDIR, std::system_category()) } << error_path{ newpath });
				}
				else
				{
					// newpath exists and is not a dir
					resolve_name_conflict();
				}
			}
		}
		else if (newpath.string().back() == '/')
		{
			FLEXFS_THROW(system_exception{ std::error_code(ENOTDIR, std::system_category()) } << error_path{ newpath });
		}
		else
		{
			resolve_name_conflict();
		}
	}
	else
	{
		// newpath does not exist
		if (newpath.string().back() == '/')
		{
			newpath /= source.orig_path.filename();
		}
		if (newpath.has_parent_path())
		{
			const auto parent = newpath.parent_path();
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
	return newpath;
}

} // namespace

void move_file(std::shared_ptr<i_access> access, source& source, const destination& dest)
{
	auto newpath = make_dest_path(access, source, dest);
	access->rename(source.current_path, newpath);
	source.current_path = newpath;
}

fspath copy_file(std::shared_ptr<i_access>                       source_access,
                 const source&                                   source,
                 std::shared_ptr<i_access>                       dest_access,
                 const destination&                              dest,
                 std::function<void(std::uint64_t bytes_copied)> on_progress)
{
	auto in = source_access->open(source.current_path, O_RDONLY | O_BINARY, 0);

	const auto destpath = make_dest_path(dest_access, source, dest);

	auto out = dest_access->open(destpath, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, source_access->stat(source.current_path).get_mode());

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

	return destpath;
}

} // namespace flexfs
