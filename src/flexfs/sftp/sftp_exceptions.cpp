#include "flexfs/sftp/sftp_exceptions.h"

namespace flexfs {
namespace sftp {

namespace {

const char* get_sftp_error_code_name(int err)
{
#define SFTP_ERROR_CODES(_)                                                                                                                \
	_(SSH_FX_OK)                                                                                                                           \
	_(SSH_FX_EOF)                                                                                                                          \
	_(SSH_FX_NO_SUCH_FILE)                                                                                                                 \
	_(SSH_FX_PERMISSION_DENIED)                                                                                                            \
	_(SSH_FX_FAILURE)                                                                                                                      \
	_(SSH_FX_BAD_MESSAGE)                                                                                                                  \
	_(SSH_FX_NO_CONNECTION)                                                                                                                \
	_(SSH_FX_CONNECTION_LOST)                                                                                                              \
	_(SSH_FX_OP_UNSUPPORTED)                                                                                                               \
	_(SSH_FX_INVALID_HANDLE)                                                                                                               \
	_(SSH_FX_NO_SUCH_PATH)                                                                                                                 \
	_(SSH_FX_FILE_ALREADY_EXISTS)                                                                                                          \
	_(SSH_FX_WRITE_PROTECT)                                                                                                                \
	_(SSH_FX_NO_MEDIA)                                                                                                                     \
	// SFT_ERROR_CODES

	switch (err)
	{
#undef expand
#define expand(x)                                                                                                                          \
	case x:                                                                                                                                \
		return #x;
		SFTP_ERROR_CODES(expand) //
	default:
		return "UNKNOWN";
	}
}

} // namespace

ssh_exception::ssh_exception(ssh_session session)
{
	*this << error_mesg(ssh_get_error(session));
	*this << ssh_error_code(ssh_get_error_code(session));
}

sftp_exception::sftp_exception(ssh_session ssh, sftp_session sftp)
    : ssh_exception(ssh)
{
	const auto err = sftp_get_error(sftp);
	*this << sftp_error_code(err);
	*this << sftp_error_code_name(get_sftp_error_code_name(err));
}

sftp_exception::sftp_exception(sftp_session sftp)
    : ssh_exception{}
{
	const auto err = sftp_get_error(sftp);
	*this << sftp_error_code(err);
	*this << sftp_error_code_name(get_sftp_error_code_name(err));
}

sftp_exception::sftp_exception(std::shared_ptr<session> session)
    : ssh_exception(session->ssh())
{
	const auto err = sftp_get_error(session->sftp());
	*this << sftp_error_code(err);
	*this << sftp_error_code_name(get_sftp_error_code_name(err));
}

} // namespace sftp
} // namespace flexfs
