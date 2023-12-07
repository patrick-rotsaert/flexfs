#pragma once

#include "flexfs/core/i_interruptor.h"

namespace flexfs {

class noop_interruptor final : public i_interruptor
{
	void interrupt() override;
	bool is_interrupted() override;
	bool wait_for_interruption(std::chrono::milliseconds duration) override;
};

} // namespace flexfs
