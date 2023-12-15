//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/sftp/sftp_watcher.h"
#include "flexfs/core/logging.h"
#include "flexfs/core/formatters.h"
#include <map>

namespace flexfs {
namespace sftp {

class watcher::impl final
{
	fspath                          dir_;
	std::uint32_t                   scan_interval_ms_;
	std::shared_ptr<i_access>       access_;
	std::shared_ptr<i_interruptor>  interruptor_;
	std::map<std::string, direntry> files_;

public:
	impl(const fspath& dir, std::uint32_t scan_interval_ms, std::shared_ptr<i_access> access, std::shared_ptr<i_interruptor> interruptor)
	    : dir_{ dir }
	    , scan_interval_ms_{ scan_interval_ms }
	    , access_{ access }
	    , interruptor_{ interruptor }
	    , files_{ list_files() }
	{
		fslog(debug, "watching {}", this->dir_);
		fslog(trace, "initial file count = {}", this->files_.size());
	}

	std::vector<direntry> watch()
	{
		std::vector<direntry> result;

		fslog(trace, "wait for interruption during {} ms", this->scan_interval_ms_);
		if (this->interruptor_->wait_for_interruption(std::chrono::milliseconds{ this->scan_interval_ms_ }))
		{
			BOOST_THROW_EXCEPTION(interrupted_exception{});
		}

		auto files = this->list_files();
		fslog(trace, "current file count = {}, previous file count = {}", files.size(), this->files_.size());

		for (const auto& pair : files)
		{
			const auto& file = pair.second;
			const auto  it   = this->files_.find(file.name);
			if (it == this->files_.end())
			{
				result.push_back(file);
			}
		}

		this->files_.swap(files);

		return result;
	}

private:
	std::map<std::string, direntry> list_files()
	{
		auto result = std::map<std::string, direntry>{};

		// get all entries in directory
		const auto files = this->access_->ls(this->dir_);

		// convert to map
		for (auto& entry : files)
		{
			auto name = entry.name;
			result.emplace(std::move(name), std::move(entry));
		}

		return result;
	}
};

watcher::watcher(const fspath&                  dir,
                 std::uint32_t                  scan_interval_ms,
                 std::shared_ptr<i_access>      access,
                 std::shared_ptr<i_interruptor> interruptor)
    : pimpl_{ std::make_unique<impl>(dir, scan_interval_ms, access, interruptor) }
{
}

watcher::~watcher() noexcept
{
}

std::vector<direntry> watcher::watch()
{
	return this->pimpl_->watch();
}

} // namespace sftp
} // namespace flexfs
