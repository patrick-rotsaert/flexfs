//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/sftp/i_ssh_api.h"

namespace flexfs {
namespace sftp {

class ssh_api : public i_ssh_api
{
public:
	ssh_api();
	~ssh_api() override;

	ssh_api(ssh_api&&)      = delete;
	ssh_api(const ssh_api&) = delete;

	ssh_api& operator=(ssh_api&&)      = delete;
	ssh_api& operator=(const ssh_api&) = delete;

	ssh_session ssh_new() override;
	void        ssh_free(ssh_session session) override;
	int         ssh_options_set(ssh_session session, enum ssh_options_e type, const void* value) override;
	int         ssh_connect(ssh_session session) override;
	void        ssh_disconnect(ssh_session session) override;
	int         ssh_get_server_publickey(ssh_session session, ssh_key* key) override;
	int         ssh_get_publickey_hash(const ssh_key key, enum ssh_publickey_hash_type type, unsigned char** hash, size_t* hlen) override;
	void        ssh_clean_pubkey_hash(unsigned char** hash) override;
	int         ssh_pki_import_privkey_base64(const char*       b64_key,
	                                          const char*       passphrase,
	                                          ssh_auth_callback auth_fn,
	                                          void*             auth_data,
	                                          ssh_key*          pkey) override;
	int         ssh_pki_export_privkey_to_pubkey(const ssh_key privkey, ssh_key* pkey) override;
	int         ssh_key_is_private(const ssh_key k) override;
	int         ssh_key_is_public(const ssh_key k) override;
	void        ssh_key_free(ssh_key key) override;
	int         ssh_userauth_none(ssh_session session, const char* username) override;
	int         ssh_userauth_list(ssh_session session, const char* username) override;
	int         ssh_userauth_try_publickey(ssh_session session, const char* username, const ssh_key pubkey) override;
	int         ssh_userauth_publickey(ssh_session session, const char* username, const ssh_key privkey) override;
	int         ssh_userauth_password(ssh_session session, const char* username, const char* password) override;
	int         ssh_set_log_callback(ssh_logging_callback cb) override;
	int         ssh_set_callbacks(ssh_session session, ssh_callbacks cb) override;
	const char* ssh_get_error(void* error) override;
	char*       ssh_get_hexa(const unsigned char* what, size_t len) override;
	void        ssh_string_free_char(char* s) override;

	sftp_session    sftp_new(ssh_session session) override;
	void            sftp_free(sftp_session sftp) override;
	int             sftp_init(sftp_session sftp) override;
	sftp_attributes sftp_stat(sftp_session session, const char* path) override;
	sftp_attributes sftp_lstat(sftp_session session, const char* path) override;
	char*           sftp_readlink(sftp_session sftp, const char* path) override;
	sftp_dir        sftp_opendir(sftp_session session, const char* path) override;
	int             sftp_closedir(sftp_dir dir) override;
	sftp_attributes sftp_readdir(sftp_session session, sftp_dir dir) override;
	sftp_file       sftp_open(sftp_session session, const char* file, int accesstype, mode_t mode) override;
	int             sftp_close(sftp_file file) override;
	ssize_t         sftp_read(sftp_file file, void* buf, size_t count) override;
	ssize_t         sftp_write(sftp_file file, const void* buf, size_t count) override;
	int             sftp_mkdir(sftp_session sftp, const char* directory, mode_t mode) override;
	int             sftp_rename(sftp_session sftp, const char* original, const char* newname) override;
	int             sftp_unlink(sftp_session sftp, const char* file) override;
	void            sftp_attributes_free(sftp_attributes file) override;
	int             sftp_dir_eof(sftp_dir dir) override;
	int             sftp_get_error(sftp_session sftp) override;
};

} // namespace sftp
} // namespace flexfs
