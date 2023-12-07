//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/core/operations.h"
#include "flexfs/core/make_dest_path.h"
#include "flexfs/core/attributes.h"
#include "flexfs/core/i_file.h"
#include <array>

namespace flexfs {

void move_file(i_access& access, source& source, const destination& dest)
{
	const auto new_path = make_dest_path(access, source, access, dest);
	access.rename(source.current_path, new_path);
	source.current_path = new_path;
}

fspath copy_file(i_access&                                       source_access,
                 const source&                                   source,
                 i_access&                                       dest_access,
                 const destination&                              dest,
                 std::function<void(std::uint64_t bytes_copied)> on_progress)
{
	auto in = source_access.open(source.current_path, O_RDONLY | O_BINARY, 0);

	const auto dest_path = make_dest_path(source_access, source, dest_access, dest);

	auto out =
	    dest_access.open(dest_path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, source_access.stat(source.current_path).get_mode() & ~S_IFMT);

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
