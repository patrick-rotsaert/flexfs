//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/sftp/sftp_file.h"
#include "flexfs/sftp/sftp_exceptions.h"
#include "flexfs/core/logging.h"

namespace flexfs {
namespace sftp {

file::file(i_ssh_api* api, sftp_file fd, const fspath& path, std::shared_ptr<session> session, std::shared_ptr<i_interruptor> interruptor)
    : api_{ api }
    , fd_{ fd }
    , path_{ path }
    , session_{ session }
    , interruptor_{ interruptor }
{
}

file::~file() noexcept
{
	fslog(trace, "sftp_close fd={}", fmt::ptr(this->fd_));
	this->api_->sftp_close(this->fd_);
}

std::size_t file::read(void* buf, std::size_t count)
{
	this->interruptor_->throw_if_interrupted();
	fslog(trace, "sftp_read fd={} count={}", fmt::ptr(this->fd_), count);
	const auto rc = this->api_->sftp_read(this->fd_, buf, count);
	if (rc < 0)
	{
		FLEXFS_THROW(sftp_exception(this->session_) << error_opname{ "sftp_read" } << error_path{ this->path_ });
	}
	return static_cast<std::size_t>(rc);
}

std::size_t file::write(const void* buf, std::size_t count)
{
	this->interruptor_->throw_if_interrupted();
	fslog(trace, "sftp_write fd={} count={}", fmt::ptr(this->fd_), count);
	const auto rc = this->api_->sftp_write(this->fd_, buf, count);
	if (rc < 0)
	{
		FLEXFS_THROW(sftp_exception(this->session_) << error_opname{ "sftp_write" } << error_path{ this->path_ });
	}
	return static_cast<std::size_t>(rc);
}

} // namespace sftp
} // namespace flexfs
