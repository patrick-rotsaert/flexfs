add_project_library(local
	SOURCES
		local_access.cpp
		local_watcher.cpp
		local_watcher.h
		local_file.cpp
		local_file.h
	PUBLIC_HEADERS
		local_access.h
	UNIT_TEST_SOURCES
		test/unit/test_local_access.cpp
	PRIVATE_DEFINITIONS
		BOOST_STACKTRACE_LINK
	PRIVATE_INCLUDE_DIRS
		${CMAKE_CURRENT_BINARY_DIR}/../..
	PUBLIC_LIBRARIES
		${PROJECT_NAME}::core
	PRIVATE_LIBRARIES
		fmt::fmt
		Boost::thread
)

if(TARGET spdlog::spdlog)
	target_link_libraries(local PUBLIC spdlog::spdlog)
endif()
