//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/sftp/ssh_server_pubkey.h"
#include "flexfs/sftp/sftp_exceptions.h"

namespace flexfs {
namespace sftp {

ssh_server_pubkey::ssh_server_pubkey(i_ssh_api* api, ssh_session session)
{
	ssh_key    key = nullptr;
	const auto rc  = api->ssh_get_server_publickey(session, &key);
	if (rc < 0)
	{
		FLEXFS_THROW(ssh_exception(session) << error_opname{ "ssh_get_server_publickey" });
	}
	this->key_ = ssh_key_ptr{ key, [api](auto k) { api->ssh_key_free(k); } };
}

ssh_key_ptr ssh_server_pubkey::key() const
{
	return this->key_;
}

} // namespace sftp
} // namespace flexfs
