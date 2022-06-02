#include "test.h"

#include <tuple>
#include <string>

bool test_tuple()
{
	__try
	{
		ktl::tuple<int, UINT32> int_tuple_pair = { 123, 456U };
		ASSERT_TRUE(std::get<0>(int_tuple_pair) == 123, "Failed to get zeroth tuple element.");
		ASSERT_TRUE(std::get<1>(int_tuple_pair) == 456U, "Failed to get first tuple element.");

		ktl::tuple<int, UINT32, UINT64> int_tuple_triple = { 678, 910U, 1112ULL };
		ASSERT_TRUE(std::get<0>(int_tuple_triple) == 678, "Failed to get zeroth tuple element.");
		ASSERT_TRUE(std::get<1>(int_tuple_triple) == 910U, "Failed to get first tuple element.");
		ASSERT_TRUE(std::get<2>(int_tuple_triple) == 1112ULL, "Failed to get second tuple element.");

		ktl::unicode_string foo_str{ L"foo" };
		ktl::tuple<ktl::unicode_string<>, int> string_int_pair{ foo_str, 0 };
		ASSERT_TRUE(std::get<0>(string_int_pair) == L"foo", "Failed to get zeroth tuple element.");
		ASSERT_TRUE(std::get<1>(string_int_pair) == int(), "Failed to get first tuple element.");

		static_assert(ktl::tuple_size_v<ktl::tuple<int, UINT32, UINT64, int>> == 4);
		static_assert(ktl::tuple_size_v<ktl::tuple<ktl::unicode_string<>, int>> == 2);

		auto& [a, b] = int_tuple_pair;
		ASSERT_TRUE(a == 123, "Unexpected structured binding output for first element.");
		ASSERT_TRUE(b == 456U, "Unexpected structured binding output for second element.");

		auto&& [c, d, e] = int_tuple_triple;
		ASSERT_TRUE(c == 678, "Unexpected structured binding output for first element.");
		ASSERT_TRUE(d == 910U, "Unexpected structured binding output for second element.");
		ASSERT_TRUE(e == 1112ULL, "Unexpected structured binding output for third element.");

		auto& [f, g] = string_int_pair;
		ASSERT_TRUE(f == foo_str, "Unexpected structured binding output for first element.");
		ASSERT_TRUE(g == 0, "Unexpected structured binding output for second element.");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::tuple!\n");
	return true;
}