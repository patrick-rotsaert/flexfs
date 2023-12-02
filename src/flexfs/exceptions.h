//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "flexfs/api.h"
#include <boost/exception/exception.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/errinfo_nested_exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/filesystem/path.hpp>
#include <exception>
#include <string>
#include <system_error>

namespace flexfs {

// clang-format off
#undef FLEXFS_ERROR_INFO_TAGS
#define FLEXFS_ERROR_INFO_TAGS(_e_) \
	_e_(error_uuid, boost::uuids::uuid) \
	_e_(error_code, std::error_code) \
	_e_(error_mesg, std::string) \
	_e_(error_path, boost::filesystem::path) \
	_e_(error_oldpath, boost::filesystem::path) \
	_e_(error_newpath, boost::filesystem::path) \
	_e_(error_opname, std::string) \
	// FLEXFS_ERROR_INFO_TAGS
// clang-format on
#undef EXPAND
#define EXPAND(name, type) using name = boost::error_info<struct name##_, type>;
FLEXFS_ERROR_INFO_TAGS(EXPAND)

class FLEXFS_EXPORT exception : public std::exception, public boost::exception
{
public:
	using mesg = error_mesg;

	exception() noexcept;
	explicit exception(std::error_code ec) noexcept;
	explicit exception(const std::string& message) noexcept;
	explicit exception(std::error_code ec, const std::string& message) noexcept;

	const char* what() const noexcept override;
};

// FIXME: naming
#define FLEXFS_EXCPT(m) dmon::exception(m)
#define FLEXFS_THROW(e) BOOST_THROW_EXCEPTION(e)
#define FLEXFS_THROW_EXCPT(m) FLEXFS_THROW(FLEXFS_EXCPT(m))
#define FLEXFS_THROW_EXCPT_NESTED(m) FLEXFS_THROW(FLEXFS_EXCPT(m) << boost::errinfo_nested_exception{ boost::current_exception() })

struct should_not_happen_exception : public exception
{
};

class system_exception : public exception
{
public:
	system_exception();
	explicit system_exception(std::error_code ec);
	explicit system_exception(const std::string& message);
	explicit system_exception(std::error_code ec, const std::string& message);

	static std::error_code getLastErrorCode() noexcept;
};

} // namespace flexfs
