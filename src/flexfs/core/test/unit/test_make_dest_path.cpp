//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "mock_access.h"
#include "flexfs/core/make_dest_path.h"
#include <gtest/gtest.h>
#include <fmt/format.h>

namespace flexfs {

TEST(MakeDestPathTests, test_dest_path_empty)
{
	auto       access = std::make_shared<nice_mock_access>();
	const auto src    = source{ "source" };
	const auto dst    = destination{ fspath{}, std::nullopt, false, destination::conflict_policy::FAIL };
	EXPECT_ANY_THROW(make_dest_path(access, src, access, dst));
}

TEST(MakeDestPathTests, test_expand_time_placeholders_with_mtime_without_placeholders)
{
	auto       source_access = std::make_shared<nice_mock_access>();
	auto       dest_access   = std::make_shared<nice_mock_access>();
	const auto src           = source{ "source" };
	const auto dst           = destination{ "destination", destination::time_expansion::UTC, false, destination::conflict_policy::FAIL };
	auto&&     make_attributes_lambda = [] {
        auto a  = attributes{};
        a.mtime = std::chrono::system_clock::from_time_t(0);
        return a;
	};
	EXPECT_CALL(*source_access, stat(testing::Eq(src.current_path))).WillOnce(testing::Return(make_attributes_lambda()));
	EXPECT_EQ(make_dest_path(source_access, src, dest_access, dst), dst.path);
}

TEST(MakeDestPathTests, test_expand_time_placeholders_with_mtime_with_placeholders)
{
	auto       source_access = std::make_shared<nice_mock_access>();
	auto       dest_access   = std::make_shared<nice_mock_access>();
	const auto src           = source{ "source" };
	const auto dst = destination{ "destination_{:%Y-%m-%d}", destination::time_expansion::UTC, false, destination::conflict_policy::FAIL };
	auto&&     make_attributes_lambda = [] {
        auto a  = attributes{};
        a.mtime = std::chrono::system_clock::from_time_t(0);
        return a;
	};
	EXPECT_CALL(*source_access, stat(testing::Eq(src.current_path))).WillOnce(testing::Return(make_attributes_lambda()));
	EXPECT_EQ(make_dest_path(source_access, src, dest_access, dst), "destination_1970-01-01");
}

TEST(MakeDestPathTests, test_expand_time_placeholders_without_mtime)
{
	auto       source_access = std::make_shared<nice_mock_access>();
	auto       dest_access   = std::make_shared<nice_mock_access>();
	const auto src           = source{ "source" };
	const auto dst = destination{ "destination_{:%Y-%m-%d}", destination::time_expansion::UTC, false, destination::conflict_policy::FAIL };
	auto&&     make_attributes_lambda = [] { return attributes{}; };
	EXPECT_CALL(*source_access, stat(testing::Eq(src.current_path))).WillOnce(testing::Return(make_attributes_lambda()));
	// When the attributes do not contain an mtime, time expansion is not possible.
	EXPECT_ANY_THROW(make_dest_path(source_access, src, dest_access, dst));
}

TEST(MakeDestPathTests, test_dest_exists_and_is_dir_and_file_name_exists_as_subdir)
{
	auto       source_access = std::make_shared<nice_mock_access>();
	auto       dest_access   = std::make_shared<nice_mock_access>();
	const auto src           = source{ "/foo/bar" };
	const auto dst           = destination{ "/path/to/destination", std::nullopt, false, destination::conflict_policy::FAIL };
	auto&&     make_attributes_for_dir_lambda = [] {
        auto a = attributes{};
        a.type = attributes::filetype::DIR;
        return a;
	};
	// "/path/to/destination" exists and is a directory
	EXPECT_CALL(*dest_access, try_stat(testing::Eq(dst.path))).WillOnce(testing::Return(make_attributes_for_dir_lambda()));
	// "/path/to/destination/bar" exists and is a directory
	EXPECT_CALL(*dest_access, try_stat(testing::Eq(dst.path / src.orig_path.filename())))
	    .WillOnce(testing::Return(make_attributes_for_dir_lambda()));
	// When "/path/to/destination" exists as a dir and when "/path/to/destination/bar" also exists as a directory, then an exception must be thrown.
	EXPECT_ANY_THROW(make_dest_path(source_access, src, dest_access, dst));
}

TEST(MakeDestPathTests, test_dest_exists_and_is_dir_and_file_name_exists_as_a_file)
{
	auto       source_access          = std::make_shared<nice_mock_access>();
	auto       dest_access            = std::make_shared<nice_mock_access>();
	const auto src                    = source{ "/foo/bar" };
	const auto dst                    = destination{ "/path/to/destination", std::nullopt, false, destination::conflict_policy::OVERWRITE };
	auto&&     make_attributes_lambda = [](attributes::filetype type) {
        auto a = attributes{};
        a.type = type;
        return a;
	};
	// "/path/to/destination" exists and is a directory
	EXPECT_CALL(*dest_access, try_stat(testing::Eq(dst.path))).WillOnce(testing::Return(make_attributes_lambda(attributes::filetype::DIR)));
	// "/path/to/destination/bar" exists and is a file
	EXPECT_CALL(*dest_access, try_stat(testing::Eq(dst.path / src.orig_path.filename())))
	    .WillOnce(testing::Return(make_attributes_lambda(attributes::filetype::REG)));
	// When "/path/to/destination" exists as a dir and when "/path/to/destination/bar" also exists as a file and conflict policy says to overwrite the
	// destination, then we expect "/path/to/destination/bar" back as the result.
	EXPECT_EQ(make_dest_path(source_access, src, dest_access, dst), dst.path / src.orig_path.filename());
}

TEST(MakeDestPathTests, test_dest_exists_and_is_not_dir_but_name_ends_with_path_separator)
{
	auto       source_access = std::make_shared<nice_mock_access>();
	auto       dest_access   = std::make_shared<nice_mock_access>();
	const auto src           = source{ "/foo/bar" };
	const auto dst           = destination{
        fmt::format("/path/to/destination{}", fspath::preferred_separator), std::nullopt, false, destination::conflict_policy::OVERWRITE
	};
	auto&& make_attributes_lambda = [](attributes::filetype type) {
		auto a = attributes{};
		a.type = type;
		return a;
	};
	// "/path/to/destination" exists and is a file
	EXPECT_CALL(*dest_access, try_stat(testing::Eq(dst.path)))
	    .WillOnce(testing::Return(make_attributes_lambda(attributes::filetype::FILE)));
	// When "/path/to/destination" exists but not as a dir and when the path ends with a path separator, then an exception is expected.
	EXPECT_ANY_THROW(make_dest_path(source_access, src, dest_access, dst));
}

TEST(MakeDestPathTests, test_dest_exists_and_is_not_dir_and_name_does_not_end_with_path_separator)
{
	auto       source_access          = std::make_shared<nice_mock_access>();
	auto       dest_access            = std::make_shared<nice_mock_access>();
	const auto src                    = source{ "/foo/bar" };
	const auto dst                    = destination{ "/path/to/destination", std::nullopt, false, destination::conflict_policy::OVERWRITE };
	auto&&     make_attributes_lambda = [](attributes::filetype type) {
        auto a = attributes{};
        a.type = type;
        return a;
	};
	// "/path/to/destination" exists and is a file
	EXPECT_CALL(*dest_access, try_stat(testing::Eq(dst.path)))
	    .WillOnce(testing::Return(make_attributes_lambda(attributes::filetype::FILE)));
	// When "/path/to/destination" exists but not as a dir and conflict policy says to overwrite the
	// destination, then we expect "/path/to/destination" back as the result.
	EXPECT_EQ(make_dest_path(source_access, src, dest_access, dst), dst.path);
}

TEST(MakeDestPathTests, test_dest_does_not_exist_and_name_does_not_end_with_path_separator_and_has_no_parent)
{
	auto       source_access = std::make_shared<nice_mock_access>();
	auto       dest_access   = std::make_shared<nice_mock_access>();
	const auto src           = source{ "/foo/bar" };
	const auto dst           = destination{ "destination", std::nullopt, false, destination::conflict_policy::FAIL };
	// "destination" does not exist
	EXPECT_CALL(*dest_access, try_stat(testing::Eq(dst.path))).WillOnce(testing::Return(std::nullopt));
	EXPECT_EQ(make_dest_path(source_access, src, dest_access, dst), dst.path);
}

TEST(MakeDestPathTests, test_dest_does_not_exist_and_name_ends_with_path_separator_and_parents_must_be_created)
{
	auto       source_access = std::make_shared<nice_mock_access>();
	auto       dest_access   = std::make_shared<nice_mock_access>();
	const auto src           = source{ "/foo/bar" };
	const auto dst = destination{ "destination/", std::nullopt, true /*parents_must_be_created*/, destination::conflict_policy::FAIL };
	// "destination" does not exist
	EXPECT_CALL(*dest_access, try_stat(testing::Eq(dst.path))).WillOnce(testing::Return(std::nullopt));
	EXPECT_CALL(*dest_access, mkdir(testing::Eq(fspath{ "destination" }), true)).Times(1);
	EXPECT_EQ(make_dest_path(source_access, src, dest_access, dst), dst.path / src.orig_path.filename());
}

TEST(MakeDestPathTests, test_dest_does_not_exist_and_name_ends_with_path_separator_and_parents_must_not_be_created)
{
	auto       source_access = std::make_shared<nice_mock_access>();
	auto       dest_access   = std::make_shared<nice_mock_access>();
	const auto src           = source{ "/foo/bar" };
	const auto dst = destination{ "destination/", std::nullopt, false /*parents_must_not_be_created*/, destination::conflict_policy::FAIL };
	// "destination" does not exist
	EXPECT_CALL(*dest_access, try_stat(testing::Eq(dst.path))).WillOnce(testing::Return(std::nullopt));
	EXPECT_ANY_THROW(make_dest_path(source_access, src, dest_access, dst));
}

TEST(MakeDestPathTests, test_conflict_policy_overwrite)
{
	auto       source_access          = std::make_shared<nice_mock_access>();
	auto       dest_access            = std::make_shared<nice_mock_access>();
	const auto src                    = source{ "/foo/bar" };
	const auto dst                    = destination{ "/path/to/bar", std::nullopt, false, destination::conflict_policy::OVERWRITE };
	auto&&     make_attributes_lambda = [](attributes::filetype type) {
        auto a = attributes{};
        a.type = type;
        return a;
	};
	// "/path/to/bar" exists and is a file
	EXPECT_CALL(*dest_access, try_stat(testing::Eq(dst.path)))
	    .WillOnce(testing::Return(make_attributes_lambda(attributes::filetype::FILE)));
	EXPECT_EQ(make_dest_path(source_access, src, dest_access, dst), dst.path);
}

TEST(MakeDestPathTests, test_conflict_policy_autorename_without_extension)
{
	auto       source_access          = std::make_shared<nice_mock_access>();
	auto       dest_access            = std::make_shared<nice_mock_access>();
	const auto src                    = source{ "/foo/bar" };
	const auto dst                    = destination{ "/path/to/bar", std::nullopt, false, destination::conflict_policy::AUTORENAME };
	auto&&     make_attributes_lambda = [](attributes::filetype type) {
        auto a = attributes{};
        a.type = type;
        return a;
	};
	auto sq = testing::InSequence{};
	// "/path/to/bar" exists and is a file
	EXPECT_CALL(*dest_access, try_stat(testing::Eq(dst.path)))
	    .WillOnce(testing::Return(make_attributes_lambda(attributes::filetype::FILE)));
	// "/path/to/bar~1" exists
	EXPECT_CALL(*dest_access, exists(testing::Eq(dst.path.parent_path() / "bar~1"))).WillOnce(testing::Return(true));
	// "/path/to/bar~2" exists
	EXPECT_CALL(*dest_access, exists(testing::Eq(dst.path.parent_path() / "bar~2"))).WillOnce(testing::Return(true));
	// "/path/to/bar~3" does not exist
	EXPECT_CALL(*dest_access, exists(testing::Eq(dst.path.parent_path() / "bar~3"))).WillOnce(testing::Return(false));
	EXPECT_EQ(make_dest_path(source_access, src, dest_access, dst), dst.path.parent_path() / "bar~3");
}

TEST(MakeDestPathTests, test_conflict_policy_autorename_with_extension)
{
	auto       source_access          = std::make_shared<nice_mock_access>();
	auto       dest_access            = std::make_shared<nice_mock_access>();
	const auto src                    = source{ "/foo/bar" };
	const auto dst                    = destination{ "/path/to/bar.baz", std::nullopt, false, destination::conflict_policy::AUTORENAME };
	auto&&     make_attributes_lambda = [](attributes::filetype type) {
        auto a = attributes{};
        a.type = type;
        return a;
	};
	auto sq = testing::InSequence{};
	// "/path/to/bar.baz" exists and is a file
	EXPECT_CALL(*dest_access, try_stat(testing::Eq(dst.path)))
	    .WillOnce(testing::Return(make_attributes_lambda(attributes::filetype::FILE)));
	// "/path/to/bar~1.baz" exists
	EXPECT_CALL(*dest_access, exists(testing::Eq(dst.path.parent_path() / "bar~1.baz"))).WillOnce(testing::Return(true));
	// "/path/to/bar~2.baz" exists
	EXPECT_CALL(*dest_access, exists(testing::Eq(dst.path.parent_path() / "bar~2.baz"))).WillOnce(testing::Return(true));
	// "/path/to/bar~3.baz" does not exist
	EXPECT_CALL(*dest_access, exists(testing::Eq(dst.path.parent_path() / "bar~3.baz"))).WillOnce(testing::Return(false));
	EXPECT_EQ(make_dest_path(source_access, src, dest_access, dst), dst.path.parent_path() / "bar~3.baz");
}

TEST(MakeDestPathTests, test_conflict_policy_fail)
{
	auto       source_access          = std::make_shared<nice_mock_access>();
	auto       dest_access            = std::make_shared<nice_mock_access>();
	const auto src                    = source{ "/foo/bar" };
	const auto dst                    = destination{ "/path/to/bar", std::nullopt, false, destination::conflict_policy::FAIL };
	auto&&     make_attributes_lambda = [](attributes::filetype type) {
        auto a = attributes{};
        a.type = type;
        return a;
	};
	// "/path/to/bar" exists and is a file
	EXPECT_CALL(*dest_access, try_stat(testing::Eq(dst.path)))
	    .WillOnce(testing::Return(make_attributes_lambda(attributes::filetype::FILE)));
	EXPECT_ANY_THROW(make_dest_path(source_access, src, dest_access, dst));
}

} // namespace flexfs
