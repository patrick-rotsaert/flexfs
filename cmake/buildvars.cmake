#
# Copyright (C) 2023 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

if(NOT CMAKE_BUILD_TYPE)
	message(FATAL_ERROR "CMAKE_BUILD_TYPE must be set")
endif()
string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LC)

if(CMAKE_BUILD_TYPE_LC MATCHES "^debug")
	set(${PROJECT_NAME_UC}_DEBUG_BUILD 1)
else()
	set(${PROJECT_NAME_UC}_DEBUG_BUILD 0)
endif()

# Set the component names
# These are prefixed with ${PROJECT_NAME} to avoid name clashes with component names
# of projects that include this library as a subdirectory.
set(COMPONENT_RUNTIME ${PROJECT_NAME}_runtime)
set(COMPONENT_DEVELOPMENT ${PROJECT_NAME}_development)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
