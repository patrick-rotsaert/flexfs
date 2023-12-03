#
# Copyright (C) 2023 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

if(    TARGET Boost::system
   AND TARGET Boost::filesystem
   AND TARGET Boost::date_time
   )
	message(TRACE "Boost targets already defined")
else()
	find_package(Boost REQUIRED COMPONENTS system filesystem date_time)
endif()

include(FetchContent)

# {fmtlib}
# If this project is included as a subdirectory, the fmt::fmt target may already be defined.
if(NOT TARGET fmt::fmt)
	# Prefer the system package
	find_package(fmt QUIET)
	if(NOT fmt_FOUND)
		set(FMT_INSTALL "${${PROJECT_NAME_UC}_INSTALL}")
		FetchContent_Declare(fmt
		  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
		  GIT_TAG 10.0.0
		)
		FetchContent_MakeAvailable(fmt)
	endif()
endif()

# spdlog
if(${PROJECT_NAME_UC}_USE_SPDLOG)
	# If this project is included as a subdirectory, the spdlog::spdlog target may already be defined.
	if(NOT TARGET spdlog::spdlog)
		# Prefer the system package
		find_package(spdlog QUIET)
		if(NOT spdlog_FOUND)
			set(SPDLOG_FMT_EXTERNAL ON)
			set(SPDLOG_INSTALL "${${PROJECT_NAME_UC}_INSTALL}")
			FetchContent_Declare(spdlog
			  GIT_REPOSITORY https://github.com/gabime/spdlog.git
			  GIT_TAG v1.12.0
			)
			FetchContent_MakeAvailable(spdlog)
		endif()
	endif()
endif()

add_project_library(core
	SOURCES
		i_access.cpp
		i_interruptor.cpp
		attributes.cpp
		source.cpp
		destination.cpp
		operations.cpp
		exceptions.cpp
		i_logger.cpp
		logging.cpp
		formatters.h
		uuid.cpp
		uuid.h
		config.h.in
		version.cpp
		version.h.in
	PUBLIC_HEADERS
		api.h
		i_access.h
		i_interruptor.h
		direntry.h
		attributes.h
		i_watcher.h
		fspath.h
		source.h
		destination.h
		operations.h
		i_file.h
		exceptions.h
		log_level.h
		i_logger.h
		logging.h
	UNIT_TEST_SOURCES
		test/unit/test_attributes.cpp
		test/unit/test_destination.cpp
	PRIVATE_LIBRARIES
		fmt::fmt
		Boost::filesystem
)

if(TARGET spdlog::spdlog)
	target_sources(core_objects PRIVATE spdlog_logger.cpp spdlog_logger.h)
	target_link_libraries(core PUBLIC spdlog::spdlog)
	if(${PROJECT_NAME_UC}_INSTALL)
		install_project_header(spdlog_logger.h ${PROJECT_SRC_DIR})
	endif()
endif()

#
# Build time logging levels

# Values are as in log_level.h
set(LOGGING_LEVELS trace debug info warn err critical off)

if(${PROJECT_NAME_UC}_DEBUG_BUILD)
	set(DEFAULT_LOGGING_LEVEL trace)
else()
	set(DEFAULT_LOGGING_LEVEL warn)
endif()

# Logging verbosity of the library
set(${PROJECT_NAME_UC}_LOGGING_LEVEL ${DEFAULT_LOGGING_LEVEL} CACHE STRING "Logging verbosity of the ${PROJECT_NAME} library")
set_property(CACHE ${PROJECT_NAME_UC}_LOGGING_LEVEL PROPERTY STRINGS ${LOGGING_LEVELS})

configure_file(
	version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/version.h
)

configure_file(
	config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)

if(${PROJECT_NAME_UC}_INSTALL)
	install_project_header(${CMAKE_CURRENT_BINARY_DIR}/version.h ${PROJECT_BIN_SRC_DIR})
	install_project_header(${CMAKE_CURRENT_BINARY_DIR}/config.h ${PROJECT_BIN_SRC_DIR})
endif()
