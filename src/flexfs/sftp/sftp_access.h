//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/sftp/sftp_options.h"
#include "flexfs/sftp/i_ssh_knownhosts.h"
#include "flexfs/sftp/i_ssh_identity_factory.h"
#include "flexfs/core/i_access.h"
#include "flexfs/core/i_interruptor.h"
#include "flexfs/core/api.h"
#include <optional>
#include <memory>

namespace flexfs {
namespace sftp {

class FLEXFS_EXPORT access final : public i_access, public std::enable_shared_from_this<access>
{
	class impl;
	std::shared_ptr<impl> pimpl_;

public:
	explicit access(const options&                          opts,
	                std::shared_ptr<i_ssh_known_hosts>      known_hosts,
	                std::shared_ptr<i_ssh_identity_factory> ssh_identity_factory,
	                std::shared_ptr<i_interruptor>          interruptor);
	~access() noexcept;

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
};

} // namespace sftp
} // namespace flexfs
