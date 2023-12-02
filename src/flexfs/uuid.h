#pragma once

#include <boost/uuid/uuid.hpp>

namespace flexfs {

class uuid
{
public:
	static boost::uuids::uuid generate();
};

} // namespace flexfs
