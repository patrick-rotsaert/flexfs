//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/sftp/sftp_session.h"
#include "flexfs/core/i_file.h"
#include "flexfs/core/i_interruptor.h"
#include "flexfs/core/fspath.h"
#include "flexfs/core/api.h"
#include <memory>
#include <cstddef>

namespace flexfs {
namespace sftp {

class FLEXFS_EXPORT file final : public i_file
{
	sftp_file                      fd_;
	fspath                         path_;
	std::shared_ptr<session>       session_;
	std::shared_ptr<i_interruptor> interruptor_;

public:
	explicit file(sftp_file fd, const fspath& path, std::shared_ptr<session> session, std::shared_ptr<i_interruptor> interruptor);
	~file() noexcept;

	std::size_t read(void* buf, std::size_t count) override;
	std::size_t write(const void* buf, std::size_t count) override;
};

} // namespace sftp
} // namespace flexfs
