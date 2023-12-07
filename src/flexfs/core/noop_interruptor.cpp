#include "flexfs/core/noop_interruptor.h"

namespace flexfs {

void noop_interruptor::interrupt()
{
}

bool noop_interruptor::is_interrupted()
{
	return false;
}

bool noop_interruptor::wait_for_interruption(std::chrono::milliseconds /*duration*/)
{
	return false;
}

} // namespace flexfs
