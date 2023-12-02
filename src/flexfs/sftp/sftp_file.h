#pragma once

#include "flexfs/sftp/sftp_session.h"
#include "flexfs/i_file.h"
#include "flexfs/i_interruptor.h"
#include "flexfs/fspath.h"
#include "flexfs/api.h"
#include <memory>
#include <cstddef>

namespace flexfs {
namespace sftp {

class FLEXFS_EXPORT file : public i_file
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
