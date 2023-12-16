//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/sftp/ssh_public_key.h"
#include "flexfs/sftp/sftp_exceptions.h"

namespace flexfs {
namespace sftp {

ssh_public_key::ssh_public_key(i_ssh_api* api, const ssh_private_key& pkey)
{
	auto       key = ssh_key{ nullptr };
	const auto rc  = api->ssh_pki_export_privkey_to_pubkey(pkey.key().get(), &key);
	if (rc != SSH_OK)
	{
		FLEXFS_THROW(ssh_exception{} << error_opname{ "ssh_pki_export_privkey_to_pubkey" });
	}
	if (!api->ssh_key_is_public(key))
	{
		FLEXFS_THROW(ssh_exception{} << error_mesg{ "Not a public key" } << error_opname{ "ssh_key_is_public" });
	}
	this->key_ = ssh_key_ptr{ key, [api](auto k) { api->ssh_key_free(k); } };
}

ssh_key_ptr ssh_public_key::key() const
{
	return this->key_;
}

} // namespace sftp
} // namespace flexfs
