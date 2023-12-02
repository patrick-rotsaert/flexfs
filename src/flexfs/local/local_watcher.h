#pragma once

#include "flexfs/core/api.h"
#include "flexfs/core/i_watcher.h"
#include "flexfs/core/fspath.h"
#include <memory>

namespace flexfs {
namespace local {

class FLEXFS_EXPORT watcher : public i_watcher
{
private:
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit watcher(const fspath& dir, int cancelfd);
	~watcher() noexcept;

	std::vector<direntry> watch() override;
};

} // namespace local
} // namespace flexfs
