#pragma once

#include "flexfs/sftp/sftp_access.h"
#include "flexfs/core/i_watcher.h"
#include "flexfs/core/fspath.h"
#include "flexfs/core/api.h"
#include <memory>
#include <cstddef>

namespace flexfs {
namespace sftp {

class FLEXFS_EXPORT watcher : public i_watcher
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit watcher(const fspath&                  dir,
	                 std::uint32_t                  scan_interval_ms,
	                 std::shared_ptr<access>        access,
	                 std::shared_ptr<i_interruptor> interruptor);
	~watcher() noexcept;

	std::vector<direntry> watch() override;
};

} // namespace sftp
} // namespace flexfs
