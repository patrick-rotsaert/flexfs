#pragma once

#include "flexfs/api.h"
#include <string>
#include <optional>
#include <cstddef>

namespace flexfs {
namespace sftp {

struct FLEXFS_EXPORT options
{
	std::string                  host;
	std::optional<std::uint16_t> port;
	std::string                  user;
	std::optional<std::string>   password;

	// known-hosts policy
	bool allow_unknown_host_key = true;  // if true, then unknown host keys will be saved in the database.
	bool allow_changed_host_key = false; // dangerous!

	std::uint32_t watcher_scan_interval_ms = 5000;

	enum class ssh_log_level
	{
		NOLOG,
		WARNING,
		PROTOCOL,
		PACKET,
		FUNCTIONS
	};
	ssh_log_level ssh_logging_verbosity = ssh_log_level::NOLOG;
};

} // namespace sftp
} // namespace flexfs
