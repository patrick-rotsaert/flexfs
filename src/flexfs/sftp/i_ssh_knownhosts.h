#pragma once

#include "flexfs/api.h"
#include <string>

namespace flexfs {
namespace sftp {

class FLEXFS_EXPORT i_ssh_known_hosts
{
public:
	enum class result
	{
		KNOWN,
		UNKNOWN,
		CHANGED
	};

public:
	virtual ~i_ssh_known_hosts() noexcept = default;

	virtual result verify(const std::string& host, const std::string& pubkey_hash)  = 0;
	virtual void   persist(const std::string& host, const std::string& pubkey_hash) = 0;
};

} // namespace sftp
} // namespace flexfs
