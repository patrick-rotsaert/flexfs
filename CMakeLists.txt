#
# Copyright (C) 2023 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 3.19)
project(flexfs VERSION 0.1.0 LANGUAGES CXX)

# Note to self: include this as early as possible
include(GNUInstallDirs)

enable_testing()

string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UC)
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" ${PROJECT_NAME_UC}_IS_TOP_LEVEL)

option(${PROJECT_NAME_UC}_BUILD_EXAMPLES "Build the examples" "${${PROJECT_NAME_UC}_IS_TOP_LEVEL}")
option(${PROJECT_NAME_UC}_TEST "Build the tests" "${${PROJECT_NAME_UC}_IS_TOP_LEVEL}")
option(${PROJECT_NAME_UC}_RUN_UNIT_TESTS_ON_BUILD "Run the unit tests during build" "${${PROJECT_NAME_UC}_IS_TOP_LEVEL}")
option(${PROJECT_NAME_UC}_INSTALL "Include install rules for ${PROJECT_NAME}" "${${PROJECT_NAME_UC}_IS_TOP_LEVEL}")
option(${PROJECT_NAME_UC}_CPACK "Include cpack rules for ${PROJECT_NAME}" "${${PROJECT_NAME_UC}_IS_TOP_LEVEL}")

option(${PROJECT_NAME_UC}_USE_SPDLOG "Use spdlog as the default logger in the ${PROJECT_NAME} library" "${${PROJECT_NAME_UC}_IS_TOP_LEVEL}")

if(${PROJECT_NAME_UC}_INSTALL)
	# When install rules are included, then the fmt and spdlog packages must be pre-installed.
	# If these libraries are not pre-installed, then they will be fetched from github (see src/fs/CMakeLists.txt)
	# and then their install targets would be included, which is probably not what you want.
	find_package(fmt REQUIRED)
	if(${PROJECT_NAME_UC}_USE_SPDLOG)
		find_package(spdlog REQUIRED)
	endif()
endif()

# Let ${PROJECT_NAME}_SHARED_LIBS override BUILD_SHARED_LIBS
if(DEFINED ${PROJECT_NAME_UC}_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "${${PROJECT_NAME_UC}_SHARED_LIBS}")
endif()

include(cmake/buildvars.cmake)
include(cmake/paths.cmake)
include(cmake/version.cmake)
include(cmake/gtest.cmake)
include(cmake/functions.cmake)

add_subdirectory(src)

if(${PROJECT_NAME_UC}_INSTALL)
	add_subdirectory(install)
	if(${PROJECT_NAME_UC}_CPACK)
		add_subdirectory(cpack)
	endif()
endif()
