#pragma once

#include "flexfs/sftp/sftp_session.h"
#include "flexfs/exceptions.h"
#include "flexfs/api.h"
#include <string>
#include <libssh/libssh.h>
#include <libssh/sftp.h>

namespace flexfs {
namespace sftp {

struct FLEXFS_EXPORT ssh_exception : public exception
{
	using ssh_error_code = boost::error_info<struct ssh_error_code_, int>;
	using exception::exception;
	explicit ssh_exception(ssh_session session);
};

struct FLEXFS_EXPORT ssh_host_key_exception : public ssh_exception
{
	using ssh_exception::ssh_exception;

	using ssh_host             = boost::error_info<struct ssh_host_, std::string>;
	using ssh_host_pubkey_hash = boost::error_info<struct ssh_host_pubkey_hash_, std::string>;
};

struct FLEXFS_EXPORT ssh_host_key_unknown : public ssh_host_key_exception
{
	using ssh_host_key_exception::ssh_host_key_exception;

	using ssh_host_key_exception::ssh_host;
	using ssh_host_key_exception::ssh_host_pubkey_hash;
};

struct FLEXFS_EXPORT ssh_host_key_changed : public ssh_host_key_exception
{
	using ssh_host_key_exception::ssh_host_key_exception;

	using ssh_host_key_exception::ssh_host;
	using ssh_host_key_exception::ssh_host_pubkey_hash;
};

struct FLEXFS_EXPORT ssh_auth_exception : public ssh_exception
{
	using ssh_exception::ssh_exception;
};

struct FLEXFS_EXPORT sftp_exception : public ssh_exception
{
	using sftp_error_code      = boost::error_info<struct sftp_error_code_, int>;
	using sftp_error_code_name = boost::error_info<struct sftp_error_code_name_, const char*>;
	using ssh_exception::ssh_exception;
	explicit sftp_exception(ssh_session ssh, sftp_session sftp);
	explicit sftp_exception(sftp_session sftp);
	explicit sftp_exception(std::shared_ptr<session> session);
};

} // namespace sftp
} // namespace flexfs
