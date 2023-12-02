#include "flexfs/core/uuid.h"
#include <boost/uuid/uuid_generators.hpp>

namespace flexfs {

boost::uuids::uuid uuid::generate()
{
	thread_local auto gen = boost::uuids::random_generator{};
	return gen();
}

} // namespace flexfs
