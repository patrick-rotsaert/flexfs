//
// Copyright (C) 2023 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "mock_access.h"
#include "mock_file.h"
#include "flexfs/core/operations.h"
#include <gtest/gtest.h>
#include <cstring>

namespace flexfs {

TEST(OperationsTests, test_move_file)
{
	auto       access = nice_mock_access{};
	auto       src    = source{ "source" };
	const auto dst    = destination{ "destination", std::nullopt, false, destination::conflict_policy::FAIL };
	// "destination" does not exist
	// This expected call is done by make_dest_path
	EXPECT_CALL(access, try_stat(testing::Eq(dst.path))).Times(1).WillOnce(testing::Return(std::nullopt));
	EXPECT_CALL(access, rename(testing::Eq(src.current_path), testing::Eq(dst.path))).Times(1);
	move_file(access, src, dst);
	EXPECT_EQ(src.orig_path, fspath{ "source" });
	EXPECT_EQ(src.current_path, fspath{ "destination" });
}

MATCHER_P2(BufferEq, expected, size, "")
{
	const auto ptr    = static_cast<const char*>(arg);
	const auto result = std::memcmp(ptr, expected, size);
	return result == 0;
}

TEST(OperationsTests, test_copy_file)
{
	auto sq = testing::InSequence{};

	auto source_access = nice_mock_access{};
	auto dest_access   = nice_mock_access{};

	auto       src = source{ "source" };
	const auto dst = destination{ "destination", std::nullopt, false, destination::conflict_policy::FAIL };

	auto source_file = std::make_unique<nice_mock_file>();
	auto dest_file   = std::make_unique<nice_mock_file>();

	// source_file and dest_file will be moved, need to keep a reference to them.
	auto& source_file_ref = *source_file;
	auto& dest_file_ref   = *dest_file;

	auto&& make_attributes_lambda = [] {
		auto a = attributes{};
		a.set_mode(S_IFREG | 0664);
		return a;
	};

	EXPECT_CALL(source_access, open(testing::Eq(src.current_path), O_RDONLY | O_BINARY, 0))
	    .Times(1)
	    .WillOnce(testing::Return(testing::ByMove(std::move(source_file))));
	// "destination" does not exist
	// This expected call is done by make_dest_path
	EXPECT_CALL(dest_access, try_stat(testing::Eq(dst.path))).Times(1).WillOnce(testing::Return(std::nullopt));
	EXPECT_CALL(source_access, stat(testing::Eq(src.current_path))).Times(1).WillOnce(testing::Return(make_attributes_lambda()));
	EXPECT_CALL(dest_access, open(testing::Eq(dst.path), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0664))
	    .Times(1)
	    .WillOnce(testing::Return(testing::ByMove(std::move(dest_file))));

	char           block[1024];
	constexpr auto block_size = sizeof(block);
	static_assert(block_size % 2 == 0);
	auto block_middle = &block[block_size / 2];
	std::memset(block, 0xAA, block_size / 2);
	std::memset(block_middle, 0x55, block_size / 2);

	// Read 1024 bytes
	EXPECT_CALL(source_file_ref, read(testing::NotNull(), testing::Ge(block_size)))
	    .WillOnce(testing::DoAll([block](void* buf, std::size_t /*count*/) { std::memcpy(buf, block, block_size); },
	                             testing::Return(block_size)));
	// Write 1024 bytes
	EXPECT_CALL(dest_file_ref, write(BufferEq(block, block_size), block_size)).WillOnce(testing::Return(block_size));

	// Read 1024 bytes
	EXPECT_CALL(source_file_ref, read(testing::NotNull(), testing::Ge(block_size)))
	    .WillOnce(testing::DoAll([block](void* buf, std::size_t /*count*/) { std::memcpy(buf, block, block_size); },
	                             testing::Return(block_size)));
	// Write first 512 bytes
	EXPECT_CALL(dest_file_ref, write(BufferEq(block, block_size), block_size)).WillOnce(testing::Return(block_size / 2));
	// Write last 512 bytes
	EXPECT_CALL(dest_file_ref, write(BufferEq(block_middle, block_size / 2), block_size / 2)).WillOnce(testing::Return(block_size / 2));

	EXPECT_CALL(source_file_ref, read(testing::NotNull(), testing::Ge(block_size))).WillOnce(testing::Return(0));

	EXPECT_EQ(copy_file(source_access, src, dest_access, dst, nullptr), dst.path);
}

} // namespace flexfs
