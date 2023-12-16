//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/sftp/i_ssh_api.h"
#include "flexfs/sftp/ptr_types.h"

namespace flexfs {
namespace sftp {

class ssh_connection
{
	i_ssh_api*      api_;
	ssh_session_ptr session_;

public:
	explicit ssh_connection(i_ssh_api* api, ssh_session_ptr session);

	~ssh_connection() noexcept;
};

} // namespace sftp
} // namespace flexfs
