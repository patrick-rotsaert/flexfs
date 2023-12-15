//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/api.h"
#include <string>

namespace flexfs {
namespace sftp {

class FLEXFS_EXPORT i_ssh_known_hosts
{
public:
	enum class result
	{
		KNOWN,
		UNKNOWN,
		CHANGED
	};

public:
	virtual ~i_ssh_known_hosts() noexcept;

	virtual result verify(const std::string& host, const std::string& pubkey_hash)  = 0;
	virtual void   persist(const std::string& host, const std::string& pubkey_hash) = 0;
};

} // namespace sftp
} // namespace flexfs
