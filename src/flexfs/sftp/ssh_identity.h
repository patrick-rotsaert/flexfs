#pragma once

#include "flexfs/api.h"
#include <string>

namespace flexfs {
namespace sftp {

class FLEXFS_EXPORT ssh_identity
{
public:
	std::string name;
	std::string pkey;
};

} // namespace sftp
} // namespace flexfs
