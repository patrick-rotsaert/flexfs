#
# Copyright (C) 2023 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

# libssh
# If this project is included as a subdirectory, the ssh::ssh target may already be defined.
if(NOT TARGET ssh::ssh)
	# Prefer the system package
	find_package(ssh QUIET)
	if(NOT ssh_FOUND)
		# sudo apt install libssh-dev
		find_library(LIBSSH_LIBRARY NAMES ssh)
		find_path(LIBSSH_INCLUDE_DIR NAMES libssh/libssh.h)
		if(LIBSSH_LIBRARY AND LIBSSH_INCLUDE_DIR)
			add_library(libssh INTERFACE)
			target_include_directories(libssh INTERFACE ${LIBSSH_INCLUDE_DIR})
			target_link_libraries(libssh INTERFACE ${LIBSSH_LIBRARY})
			add_library(ssh::ssh ALIAS libssh)
		else()
			set(WITH_NACL OFF)
			set(WITH_EXAMPLES OFF)
			FetchContent_Declare(ssh
			  GIT_REPOSITORY https://github.com/patrick-rotsaert/libssh.git
			  GIT_BRANCH stable-0.10
			)
			FetchContent_MakeAvailable(ssh)
		endif()
	endif()
endif()

add_project_library(sftp
	SOURCES
		i_ssh_api.cpp
		i_ssh_api.h
		ssh_api.cpp
		ssh_api.h
		sftp_access.cpp
		sftp_watcher.cpp
		sftp_watcher.h
		sftp_exceptions.cpp
		i_ssh_knownhosts.cpp
		i_ssh_identity_factory.cpp
		sftp_file.cpp
		sftp_file.h
		sftp_session.cpp
		sftp_session.h
		ssh_connection.cpp
		ssh_connection.h
		ssh_server_pubkey.cpp
		ssh_server_pubkey.h
		ssh_pubkey_hash.cpp
		ssh_pubkey_hash.h
		ssh_private_key.cpp
		ssh_private_key.h
		ssh_public_key.cpp
		ssh_public_key.h
		ptr_types.h
	PUBLIC_HEADERS
		i_ssh_knownhosts.h
		i_ssh_identity_factory.h
		ssh_identity.h
		sftp_access.h
		sftp_exceptions.h
		sftp_options.h
	PRIVATE_INCLUDE_DIRS
		${CMAKE_CURRENT_BINARY_DIR}/../..
	PUBLIC_LIBRARIES
		${PROJECT_NAME}::core
	PRIVATE_LIBRARIES
		fmt::fmt
)

if(TARGET spdlog::spdlog)
	target_link_libraries(sftp PUBLIC spdlog::spdlog)
endif()
