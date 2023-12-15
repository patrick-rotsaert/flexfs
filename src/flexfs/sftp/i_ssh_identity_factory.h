//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/sftp/ssh_identity.h"
#include "flexfs/core/api.h"
#include <memory>
#include <vector>

namespace flexfs {
namespace sftp {

class FLEXFS_EXPORT i_ssh_identity_factory
{
public:
	virtual ~i_ssh_identity_factory() noexcept;

	virtual std::vector<std::shared_ptr<ssh_identity>> create() = 0;
};

} // namespace sftp
} // namespace flexfs
