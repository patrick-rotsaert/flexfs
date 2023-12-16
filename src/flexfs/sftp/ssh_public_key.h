//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/sftp/i_ssh_api.h"
#include "flexfs/sftp/ssh_private_key.h"
#include "flexfs/sftp/ptr_types.h"
#include <string>

namespace flexfs {
namespace sftp {

class ssh_public_key
{
	ssh_key_ptr key_;

public:
	explicit ssh_public_key(i_ssh_api* api, const ssh_private_key& pkey);

	ssh_key_ptr key() const;
};

} // namespace sftp
} // namespace flexfs
