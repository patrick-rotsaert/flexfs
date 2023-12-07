#pragma once

#include "flexfs/core/api.h"
#include "flexfs/core/i_access.h"
#include "flexfs/core/i_interruptor.h"
#include <optional>

namespace flexfs {
namespace local {

class FLEXFS_EXPORT access final : public i_access
{
	std::shared_ptr<i_interruptor> interruptor_;

public:
	explicit access(std::shared_ptr<i_interruptor> interruptor);

	bool                       is_remote() const override;
	std::vector<direntry>      ls(const fspath& dir) override;
	bool                       exists(const fspath& path) override;
	std::optional<attributes>  try_stat(const fspath& path) override;
	attributes                 stat(const fspath& path) override;
	attributes                 lstat(const fspath& path) override;
	void                       remove(const fspath& path) override;
	void                       mkdir(const fspath& path, bool parents) override;
	void                       rename(const fspath& oldpath, const fspath& newpath) override;
	std::unique_ptr<i_file>    open(const fspath& path, int flags, mode_t mode) override;
	std::shared_ptr<i_watcher> create_watcher(const fspath& dir, int cancelfd) override;

	static direntry get_direntry(const fspath& path);
};

} // namespace local
} // namespace flexfs
