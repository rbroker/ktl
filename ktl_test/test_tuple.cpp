#include "test.h"

#include <tuple>
#include <string>

bool test_tuple()
{
	__try
	{
		// Compile-time checks
		static_assert(ktl::is_same_v<ktl::tuple_element<0, ktl::tuple<int, UINT32>>::type, int>);
		static_assert(ktl::is_same_v<ktl::tuple_element<1, ktl::tuple<int, UINT32>>::type, UINT32>);
		static_assert(ktl::is_same_v<ktl::tuple_element_t<0, ktl::tuple<int, UINT32>>, int>);

		static_assert(ktl::tuple_size_v<ktl::tuple<int, UINT32, UINT64, int>> == 4);
		static_assert(ktl::tuple_size_v<ktl::tuple<ktl::unicode_string<>, int>> == 2);

		// Basic tuple behaviour.
		ktl::tuple<int, UINT32> int_tuple_pair = { 123, 456U };
		ASSERT_TRUE(ktl::get<0>(int_tuple_pair) == 123, "Failed to get zeroth tuple element: %u", ktl::get<0>(int_tuple_pair));
		ASSERT_TRUE(ktl::get<1>(int_tuple_pair) == 456U, "Failed to get first tuple element: %u", ktl::get<1>(int_tuple_pair));

		ktl::tuple<int, UINT32, UINT64> int_tuple_triple = { 678, 910U, 1112ULL };
		ASSERT_TRUE(ktl::get<0>(int_tuple_triple) == 678, "Failed to get zeroth tuple element.");
		ASSERT_TRUE(ktl::get<1>(int_tuple_triple) == 910U, "Failed to get first tuple element.");
		ASSERT_TRUE(ktl::get<2>(int_tuple_triple) == 1112ULL, "Failed to get second tuple element.");

		const ktl::unicode_string c_foo_str{ L"foo" };
		ktl::tuple<ktl::unicode_string<>, int> string_int_pair{ c_foo_str, 1 };
		ASSERT_TRUE(ktl::get<0>(string_int_pair) == L"foo", "Failed to get zeroth tuple element (%wZ != %wZ)", ktl::get<0>(string_int_pair).data(), c_foo_str.data());
		ASSERT_TRUE(ktl::get<1>(string_int_pair) == 1, "Failed to get first tuple element.");

		auto& [a, b] = int_tuple_pair;
		ASSERT_TRUE(a == 123, "Unexpected structured binding output for first element.");
		ASSERT_TRUE(b == 456U, "Unexpected structured binding output for second element.");

		auto&& [c, d, e] = int_tuple_triple;
		ASSERT_TRUE(c == 678, "Unexpected structured binding output for first element.");
		ASSERT_TRUE(d == 910U, "Unexpected structured binding output for second element.");
		ASSERT_TRUE(e == 1112ULL, "Unexpected structured binding output for third element.");

		// Copy tuple!
		auto copied_tuple = string_int_pair;
		ASSERT_TRUE(ktl::get<0>(copied_tuple) == c_foo_str, "Unexpected structured binding output for first element (%wZ != %wZ)", ktl::get<0>(copied_tuple).data(), c_foo_str.data());
		ASSERT_TRUE(ktl::get<1>(copied_tuple) == 1, "Unexpected structured binding output for second element.");

		auto [f, g] = string_int_pair;
		ASSERT_TRUE(f == c_foo_str, "Unexpected structured binding output for first element (%wZ != %wZ)", f.data(), c_foo_str.data());
		ASSERT_TRUE(g == 1, "Unexpected structured binding output for second element.");

		// Move tuple...
		auto moved_tuple = ktl::move(string_int_pair);
		ASSERT_TRUE(ktl::get<0>(string_int_pair) != c_foo_str, "Unexpected value in tuple after string moved out of it");
		ASSERT_TRUE(ktl::get<0>(moved_tuple) == c_foo_str, "Unexpected structured binding output for first element.");
		ASSERT_TRUE(ktl::get<1>(moved_tuple) == 1, "Unexpected structured binding output for second element.");

		ktl::unicode_string foo_str{ L"foo" };
		ktl::unicode_string bar_str{ L"bar" };

		ktl::tuple<ktl::unicode_string<>, ktl::unicode_string<>> string_string_pair{ foo_str, ktl::move(bar_str) };
		const auto& [h, i] = string_string_pair;
		ASSERT_TRUE(h == foo_str, "Unexpected structured binding output for first element (%wZ != %wZ)", h.data(), foo_str.data());
		ASSERT_TRUE(i == L"bar", "Unexpected structured binding output for second element (%wZ != %wZ)", i.data(), foo_str.data());


	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::tuple!\n");
	return true;
}