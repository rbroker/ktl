#include "test.h"

#include <tuple>
#include <string>

int test_tuple()
{
	ktl::tuple<int, UINT32> int_tuple_pair = { 123, 456U };
	ASSERT_TRUE(ktl::get<1>(int_tuple_pair) == 456U, "Failed to get first tuple element.");
	ASSERT_TRUE(ktl::get<0>(int_tuple_pair) == 123, "Failed to get zeroth tuple element.");

	ktl::tuple<int, UINT32, UINT64> int_tuple_triple = { 678, 910U, 1112ULL };
	ASSERT_TRUE(ktl::get<0>(int_tuple_triple) == 678, "Failed to get zeroth tuple element.");
	ASSERT_TRUE(ktl::get<1>(int_tuple_triple) == 910U, "Failed to get first tuple element.");
	ASSERT_TRUE(ktl::get<2>(int_tuple_triple) == 1112ULL, "Failed to get first tuple element.");

	//ktl::tuple<ktl::unicode_string, int> string_int_pair;
	return 0;
}