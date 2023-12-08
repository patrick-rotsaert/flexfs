//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/core/api.h"
#include "flexfs/core/fspath.h"
#include <boost/system/api_config.hpp>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <fcntl.h>
#ifdef BOOST_WINDOWS_API
#include <io.h>
#endif

#ifdef BOOST_WINDOWS_API
#define O_RDONLY _O_RDONLY
#define O_WRONLY _O_WRONLY
#define O_RDWR _O_RDWR
#define O_APPEND _O_APPEND
#define O_CREAT _O_CREAT
#define O_TRUNC _O_TRUNC
#define O_EXCL _O_EXCL
#define O_TEXT _O_TEXT
#define O_BINARY _O_BINARY
#endif
#ifdef BOOST_POSIX_API
#define O_BINARY 0 // does not exist in POSIX
#endif

namespace flexfs {

class direntry;
class attributes;
class i_file;
class i_watcher;

class FLEXFS_EXPORT i_access
{
public:
	virtual ~i_access() noexcept;

	virtual bool                      is_remote() const            = 0;
	virtual std::vector<direntry>     ls(const fspath& dir)        = 0;
	virtual bool                      exists(const fspath& path)   = 0;
	virtual std::optional<attributes> try_stat(const fspath& path) = 0;
	virtual attributes                stat(const fspath& path)     = 0;
	// FIXME: add try_lstat
	virtual attributes              lstat(const fspath& path)                            = 0;
	virtual void                    remove(const fspath& path)                           = 0; // if exists(path), it is removed
	virtual void                    mkdir(const fspath& path, bool parents)              = 0;
	virtual void                    rename(const fspath& oldpath, const fspath& newpath) = 0;
	virtual std::unique_ptr<i_file> open(const fspath& path, int flags, mode_t mode)     = 0;

	/// @brief Create a directory watcher.
	/// The caller must provide a file descriptor @a cancelfd that the implementation can
	/// monitor (through select, poll, ...) for read events, e.g. the read end of a pipe.
	/// The caller can then cancel the watcher by making the file descriptor readable, e.g.
	/// by writing to the write end of the pipe.
	virtual std::shared_ptr<i_watcher> create_watcher(const fspath& dir, int cancelfd) = 0;
};

} // namespace flexfs
