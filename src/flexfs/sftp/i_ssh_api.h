//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <libssh/callbacks.h>

namespace flexfs {
namespace sftp {

class i_ssh_api
{
public:
	i_ssh_api();
	virtual ~i_ssh_api();

	i_ssh_api(i_ssh_api&&)      = delete;
	i_ssh_api(const i_ssh_api&) = delete;

	i_ssh_api& operator=(i_ssh_api&&)      = delete;
	i_ssh_api& operator=(const i_ssh_api&) = delete;

	virtual ssh_session ssh_new()                                                                                                 = 0;
	virtual void        ssh_free(ssh_session session)                                                                             = 0;
	virtual int         ssh_options_set(ssh_session session, enum ssh_options_e type, const void* value)                          = 0;
	virtual int         ssh_connect(ssh_session session)                                                                          = 0;
	virtual void        ssh_disconnect(ssh_session session)                                                                       = 0;
	virtual int         ssh_get_server_publickey(ssh_session session, ssh_key* key)                                               = 0;
	virtual int  ssh_get_publickey_hash(const ssh_key key, enum ssh_publickey_hash_type type, unsigned char** hash, size_t* hlen) = 0;
	virtual void ssh_clean_pubkey_hash(unsigned char** hash)                                                                      = 0;
	virtual int  ssh_pki_import_privkey_base64(const char*       b64_key,
	                                           const char*       passphrase,
	                                           ssh_auth_callback auth_fn,
	                                           void*             auth_data,
	                                           ssh_key*          pkey)                                                                     = 0;
	virtual int  ssh_pki_export_privkey_to_pubkey(const ssh_key privkey, ssh_key* pkey)                                           = 0;
	virtual int  ssh_key_is_private(const ssh_key k)                                                                              = 0;
	virtual int  ssh_key_is_public(const ssh_key k)                                                                               = 0;
	virtual void ssh_key_free(ssh_key key)                                                                                        = 0;
	virtual int  ssh_userauth_none(ssh_session session, const char* username)                                                     = 0;
	virtual int  ssh_userauth_list(ssh_session session, const char* username)                                                     = 0;
	virtual int  ssh_userauth_try_publickey(ssh_session session, const char* username, const ssh_key pubkey)                      = 0;
	virtual int  ssh_userauth_publickey(ssh_session session, const char* username, const ssh_key privkey)                         = 0;
	virtual int  ssh_userauth_password(ssh_session session, const char* username, const char* password)                           = 0;
	virtual int  ssh_set_log_callback(ssh_logging_callback cb)                                                                    = 0;
	virtual int  ssh_set_callbacks(ssh_session session, ssh_callbacks cb)                                                         = 0;
	virtual const char* ssh_get_error(void* error)                                                                                = 0;
	virtual char*       ssh_get_hexa(const unsigned char* what, size_t len)                                                       = 0;
	virtual void        ssh_string_free_char(char* s)                                                                             = 0;

	virtual sftp_session    sftp_new(ssh_session session)                                                  = 0;
	virtual void            sftp_free(sftp_session sftp)                                                   = 0;
	virtual int             sftp_init(sftp_session sftp)                                                   = 0;
	virtual sftp_attributes sftp_stat(sftp_session session, const char* path)                              = 0;
	virtual sftp_attributes sftp_lstat(sftp_session session, const char* path)                             = 0;
	virtual char*           sftp_readlink(sftp_session sftp, const char* path)                             = 0;
	virtual sftp_dir        sftp_opendir(sftp_session session, const char* path)                           = 0;
	virtual int             sftp_closedir(sftp_dir dir)                                                    = 0;
	virtual sftp_attributes sftp_readdir(sftp_session session, sftp_dir dir)                               = 0;
	virtual sftp_file       sftp_open(sftp_session session, const char* file, int accesstype, mode_t mode) = 0;
	virtual int             sftp_close(sftp_file file)                                                     = 0;
	virtual ssize_t         sftp_read(sftp_file file, void* buf, size_t count)                             = 0;
	virtual ssize_t         sftp_write(sftp_file file, const void* buf, size_t count)                      = 0;
	virtual int             sftp_mkdir(sftp_session sftp, const char* directory, mode_t mode)              = 0;
	virtual int             sftp_rename(sftp_session sftp, const char* original, const char* newname)      = 0;
	virtual int             sftp_unlink(sftp_session sftp, const char* file)                               = 0;
	virtual void            sftp_attributes_free(sftp_attributes file)                                     = 0;
	virtual int             sftp_dir_eof(sftp_dir dir)                                                     = 0;
	virtual int             sftp_get_error(sftp_session sftp)                                              = 0;
};

} // namespace sftp
} // namespace flexfs
