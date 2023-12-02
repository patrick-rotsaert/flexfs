#pragma once

#include "flexfs/core/api.h"
#include "flexfs/core/i_file.h"
#include "flexfs/core/i_interruptor.h"
#include "flexfs/core/fspath.h"

namespace flexfs {
namespace local {

class FLEXFS_EXPORT file : public i_file
{
	int                            fd_;
	fspath                         path_;
	std::shared_ptr<i_interruptor> interruptor_;

public:
	explicit file(int fd, const fspath& path, std::shared_ptr<i_interruptor> interruptor);
	~file() noexcept;

	std::size_t read(void* buf, std::size_t count) override;
	std::size_t write(const void* buf, std::size_t count) override;
};

} // namespace local
} // namespace flexfs
