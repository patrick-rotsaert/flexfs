//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/sftp/ssh_api.h"

namespace flexfs {
namespace sftp {

ssh_api::ssh_api()
{
}

ssh_api::~ssh_api()
{
}

ssh_session ssh_api::ssh_new()
{
	return ::ssh_new();
}

void ssh_api::ssh_free(ssh_session session)
{
	return ::ssh_free(session);
}

int ssh_api::ssh_options_set(ssh_session session, ssh_options_e type, const void* value)
{
	return ::ssh_options_set(session, type, value);
}

int ssh_api::ssh_connect(ssh_session session)
{
	return ::ssh_connect(session);
}

void ssh_api::ssh_disconnect(ssh_session session)
{
	return ::ssh_disconnect(session);
}

int ssh_api::ssh_get_server_publickey(ssh_session session, ssh_key* key)
{
	return ::ssh_get_server_publickey(session, key);
}

int ssh_api::ssh_get_publickey_hash(const ssh_key key, ssh_publickey_hash_type type, unsigned char** hash, size_t* hlen)
{
	return ::ssh_get_publickey_hash(key, type, hash, hlen);
}

void ssh_api::ssh_clean_pubkey_hash(unsigned char** hash)
{
	return ::ssh_clean_pubkey_hash(hash);
}

int ssh_api::ssh_pki_import_privkey_base64(const char*       b64_key,
                                           const char*       passphrase,
                                           ssh_auth_callback auth_fn,
                                           void*             auth_data,
                                           ssh_key*          pkey)
{
	return ::ssh_pki_import_privkey_base64(b64_key, passphrase, auth_fn, auth_data, pkey);
}

int ssh_api::ssh_pki_export_privkey_to_pubkey(const ssh_key privkey, ssh_key* pkey)
{
	return ::ssh_pki_export_privkey_to_pubkey(privkey, pkey);
}

int ssh_api::ssh_key_is_private(const ssh_key k)
{
	return ::ssh_key_is_private(k);
}

int ssh_api::ssh_key_is_public(const ssh_key k)
{
	return ::ssh_key_is_public(k);
}

void ssh_api::ssh_key_free(ssh_key key)
{
	return ::ssh_key_free(key);
}

int ssh_api::ssh_userauth_none(ssh_session session, const char* username)
{
	return ::ssh_userauth_none(session, username);
}

int ssh_api::ssh_userauth_list(ssh_session session, const char* username)
{
	return ::ssh_userauth_list(session, username);
}

int ssh_api::ssh_userauth_try_publickey(ssh_session session, const char* username, const ssh_key pubkey)
{
	return ::ssh_userauth_try_publickey(session, username, pubkey);
}

int ssh_api::ssh_userauth_publickey(ssh_session session, const char* username, const ssh_key privkey)
{
	return ::ssh_userauth_publickey(session, username, privkey);
}

int ssh_api::ssh_userauth_password(ssh_session session, const char* username, const char* password)
{
	return ::ssh_userauth_password(session, username, password);
}

int ssh_api::ssh_set_log_callback(ssh_logging_callback cb)
{
	return ::ssh_set_log_callback(cb);
}

int ssh_api::ssh_set_callbacks(ssh_session session, ssh_callbacks cb)
{
	return ::ssh_set_callbacks(session, cb);
}

const char* ssh_api::ssh_get_error(void* error)
{
	return ::ssh_get_error(error);
}

char* ssh_api::ssh_get_hexa(const unsigned char* what, size_t len)
{
	return ::ssh_get_hexa(what, len);
}

void ssh_api::ssh_string_free_char(char* s)
{
	return ::ssh_string_free_char(s);
}

sftp_session ssh_api::sftp_new(ssh_session session)
{
	return ::sftp_new(session);
}

void ssh_api::sftp_free(sftp_session sftp)
{
	return ::sftp_free(sftp);
}

int ssh_api::sftp_init(sftp_session sftp)
{
	return ::sftp_init(sftp);
}

sftp_attributes ssh_api::sftp_stat(sftp_session session, const char* path)
{
	return ::sftp_stat(session, path);
}

sftp_attributes ssh_api::sftp_lstat(sftp_session session, const char* path)
{
	return ::sftp_lstat(session, path);
}

char* ssh_api::sftp_readlink(sftp_session sftp, const char* path)
{
	return ::sftp_readlink(sftp, path);
}

sftp_dir ssh_api::sftp_opendir(sftp_session session, const char* path)
{
	return ::sftp_opendir(session, path);
}

int ssh_api::sftp_closedir(sftp_dir dir)
{
	return ::sftp_closedir(dir);
}

sftp_attributes ssh_api::sftp_readdir(sftp_session session, sftp_dir dir)
{
	return ::sftp_readdir(session, dir);
}

sftp_file ssh_api::sftp_open(sftp_session session, const char* file, int accesstype, mode_t mode)
{
	return ::sftp_open(session, file, accesstype, mode);
}

int ssh_api::sftp_close(sftp_file file)
{
	return ::sftp_close(file);
}

ssize_t ssh_api::sftp_read(sftp_file file, void* buf, size_t count)
{
	return ::sftp_read(file, buf, count);
}

ssize_t ssh_api::sftp_write(sftp_file file, const void* buf, size_t count)
{
	return ::sftp_write(file, buf, count);
}

int ssh_api::sftp_mkdir(sftp_session sftp, const char* directory, mode_t mode)
{
	return ::sftp_mkdir(sftp, directory, mode);
}

int ssh_api::sftp_rename(sftp_session sftp, const char* original, const char* newname)
{
	return ::sftp_rename(sftp, original, newname);
}

int ssh_api::sftp_unlink(sftp_session sftp, const char* file)
{
	return ::sftp_unlink(sftp, file);
}

void ssh_api::sftp_attributes_free(sftp_attributes file)
{
	return ::sftp_attributes_free(file);
}

int ssh_api::sftp_dir_eof(sftp_dir dir)
{
	return ::sftp_dir_eof(dir);
}

int ssh_api::sftp_get_error(sftp_session sftp)
{
	return ::sftp_get_error(sftp);
}

} // namespace sftp
} // namespace flexfs
