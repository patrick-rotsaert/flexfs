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

class ssh_server_pubkey
{
	ssh_key_ptr key_;

public:
	explicit ssh_server_pubkey(i_ssh_api* api, ssh_session session);

	ssh_key_ptr key() const;
};

} // namespace sftp
} // namespace flexfs
