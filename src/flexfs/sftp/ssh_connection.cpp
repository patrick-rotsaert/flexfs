//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/sftp/ssh_connection.h"
#include "flexfs/sftp/sftp_exceptions.h"

namespace flexfs {
namespace sftp {

ssh_connection::ssh_connection(i_ssh_api* api, ssh_session_ptr session)
    : api_{ api }
    , session_{ session }
{
	if (this->api_->ssh_connect(this->session_.get()) != SSH_OK)
	{
		FLEXFS_THROW(ssh_exception(this->session_.get()) << error_opname{ "ssh_connect" });
	}
}

ssh_connection::~ssh_connection() noexcept
{
	this->api_->ssh_disconnect(this->session_.get());
}

} // namespace sftp
} // namespace flexfs
