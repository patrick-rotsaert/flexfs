//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "flexfs/core/exceptions.h"
#include "flexfs/core/uuid.h"
#include <boost/exception/get_error_info.hpp>
#include <boost/exception/info.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/system/api_config.hpp>
#include <fmt/format.h>

#ifdef BOOST_WINDOWS_API
#include <Windows.h>
#else
#include <errno.h>
#endif

namespace flexfs {

exception::exception() noexcept
{
	*this << error_uuid{ uuid::generate() };
}

exception::exception(std::error_code ec) noexcept
{
	*this << error_code{ ec };
	*this << error_mesg{ ec.message() };
	*this << error_uuid{ uuid::generate() };
}

exception::exception(const std::string& message) noexcept
{
	*this << error_mesg{ message };
	*this << error_uuid{ uuid::generate() };
}

exception::exception(std::error_code ec, const std::string& message) noexcept
{
	*this << error_code{ ec };
	*this << error_mesg{ fmt::format("{}: {}", message, ec.message()) };
	*this << error_uuid{ uuid::generate() };
}

const char* exception::what() const noexcept
{
	const auto message = boost::get_error_info<mesg>(*this);
	if (message)
	{
		return message->c_str();
	}
	else
	{
		return std::exception::what();
	}
}

system_exception::system_exception()
    : exception{ getLastErrorCode() }
{
}

system_exception::system_exception(std::error_code ec)
    : exception{ ec }
{
}

system_exception::system_exception(const std::string& message)
    : exception{ getLastErrorCode(), message }
{
}

system_exception::system_exception(std::error_code ec, const std::string& message)
    : exception{ ec, message }
{
}

std::error_code system_exception::getLastErrorCode() noexcept
{
#ifdef BOOST_WINDOWS_API
	return std::error_code{ ::GetLastError(), std::system_category() };
#else
	return std::error_code{ errno, std::system_category() };
#endif
}

} // namespace flexfs
