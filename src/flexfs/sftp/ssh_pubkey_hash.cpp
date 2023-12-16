//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/sftp/ssh_pubkey_hash.h"
#include "flexfs/sftp/sftp_exceptions.h"

namespace flexfs {
namespace sftp {

ssh_pubkey_hash::ssh_pubkey_hash(i_ssh_api* api, ssh_session session, ssh_key key, ssh_publickey_hash_type type)
{
	unsigned char* hash = nullptr;
	auto           hlen = size_t{};
	const auto     rc   = api->ssh_get_publickey_hash(key, type, &hash, &hlen);
	if (rc < 0)
	{
		FLEXFS_THROW(ssh_exception(session) << error_opname{ "ssh_get_publickey_hash" });
	}
	assert(hash);
	auto hexa = api->ssh_get_hexa(hash, hlen);
	assert(hexa);
	api->ssh_clean_pubkey_hash(&hash);
	this->hash_.assign(hexa);
	api->ssh_string_free_char(hexa);
}

const std::string ssh_pubkey_hash::hash() const
{
	return this->hash_;
}

} // namespace sftp
} // namespace flexfs
