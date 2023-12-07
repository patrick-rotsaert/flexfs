#pragma once

#include "flexfs/sftp/ssh_identity.h"
#include "flexfs/core/api.h"
#include <memory>
#include <vector>

namespace flexfs {
namespace sftp {

class FLEXFS_EXPORT i_ssh_identity_factory
{
public:
	virtual ~i_ssh_identity_factory() noexcept;

	virtual std::vector<std::shared_ptr<ssh_identity>> create() = 0;
};

} // namespace sftp
} // namespace flexfs
