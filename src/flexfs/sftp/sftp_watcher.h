//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/i_access.h"
#include "flexfs/core/i_watcher.h"
#include "flexfs/core/i_interruptor.h"
#include "flexfs/core/fspath.h"
#include "flexfs/core/api.h"
#include <memory>
#include <cstddef>

namespace flexfs {
namespace sftp {

class FLEXFS_EXPORT watcher final : public i_watcher
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit watcher(const fspath&                  dir,
	                 std::uint32_t                  scan_interval_ms,
	                 std::shared_ptr<i_access>      access,
	                 std::shared_ptr<i_interruptor> interruptor);
	~watcher() noexcept;

	std::vector<direntry> watch() override;
};

} // namespace sftp
} // namespace flexfs
