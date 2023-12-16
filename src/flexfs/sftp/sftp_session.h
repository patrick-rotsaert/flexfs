//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/sftp/sftp_options.h"
#include "flexfs/sftp/i_ssh_api.h"
#include "flexfs/sftp/i_ssh_knownhosts.h"
#include "flexfs/sftp/i_ssh_identity_factory.h"
#include "flexfs/core/i_interruptor.h"
#include "flexfs/core/api.h"
#include <memory>
#include <libssh/libssh.h>
#include <libssh/sftp.h>

namespace flexfs {
namespace sftp {

class FLEXFS_EXPORT session
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit session(i_ssh_api*                              api,
	                 const options&                          opts,
	                 std::shared_ptr<i_ssh_known_hosts>      known_hosts,
	                 std::shared_ptr<i_ssh_identity_factory> ssh_identity_factory,
	                 std::shared_ptr<i_interruptor>          interruptor);
	~session() noexcept;

	ssh_session  ssh() const;
	sftp_session sftp() const;
};

} // namespace sftp
} // namespace flexfs
