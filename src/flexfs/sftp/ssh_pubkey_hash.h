//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/sftp/i_ssh_api.h"
#include <string>

namespace flexfs {
namespace sftp {

class ssh_pubkey_hash
{
	std::string hash_;

public:
	explicit ssh_pubkey_hash(i_ssh_api* api, ssh_session session, ssh_key key, ssh_publickey_hash_type type);

	const std::string hash() const;
};

} // namespace sftp
} // namespace flexfs
