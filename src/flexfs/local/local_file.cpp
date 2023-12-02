#include "flexfs/local/local_file.h"
#include "flexfs/core/exceptions.h"
#include "flexfs/core/logging.h"

#include <boost/system/api_config.hpp>

#ifdef BOOST_WINDOWS_API
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef BOOST_WINDOWS_API
#define c_close(fd) ::_close(fd)
#define c_read(fd, buf, count) ::_read(fd, buf, static_cast<unsigned int>(count))
#define c_write(fd, buf, count) ::_write(fd, buf, static_cast<unsigned int>(count))
#else
#define c_close(fd) ::close(fd)
#define c_read(fd, buf, count) ::read(fd, buf, count)
#define c_write(fd, buf, count) ::write(fd, buf, count)
#endif

namespace flexfs {
namespace local {

file::file(int fd, const fspath& path, std::shared_ptr<i_interruptor> interruptor)
    : fd_{ fd }
    , path_{ path }
    , interruptor_{ interruptor }
{
}

file::~file() noexcept
{
	fslog(trace, "close fd={}", this->fd_);
	c_close(this->fd_);
}

std::size_t file::read(void* buf, std::size_t count)
{
	this->interruptor_->throw_if_interrupted();
	fslog(trace, "read fd={} count={}", this->fd_, count);
	auto rc = c_read(this->fd_, buf, count);
	if (rc < 0)
	{
		FLEXFS_THROW(system_exception{} << error_opname{ "read" } << error_path{ this->path_ });
	}
	return static_cast<std::size_t>(rc);
}

std::size_t file::write(const void* buf, std::size_t count)
{
	this->interruptor_->throw_if_interrupted();
	fslog(trace, "write fd={} count={}", this->fd_, count);
	auto rc = c_write(this->fd_, buf, count);
	if (rc < 0)
	{
		FLEXFS_THROW(system_exception{} << error_opname{ "write" } << error_path{ this->path_ });
	}
	return static_cast<std::size_t>(rc);
}

} // namespace local
} // namespace flexfs
