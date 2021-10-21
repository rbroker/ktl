#include "test.h"

#include <set>
#include <string_view>
#include <string>
#include <vector>

using namespace ktl;

bool test_set_of_string()
{
	ktl::set<ktl::unicode_string<>> stringSet;

	// insert & grow
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"foo" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"bar" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"baz" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"hoge" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"piyo" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"fuga" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"hogera" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"toto" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"xyzzy" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"qux" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"quux" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"quuz" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"corge" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"corgi" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"corgii" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"many corgs" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"trapezium" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"a" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"b" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"c" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"d" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"$" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"¢" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"ह" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"€" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"𐍈" }), "failed to insert string into set");
	ASSERT_TRUE(stringSet.insert(ktl::unicode_string_view{ L"よろしくお願いします。" }), "failed to insert string into set");

	// find
	{
		auto it = stringSet.find(ktl::unicode_string_view{ L"$" });
		ASSERT_TRUE(it != stringSet.end(), "unable to find expected entry in set");
		ASSERT_TRUE(*it == L"$", "found entry didn't contain expected value: %wZ", it->data());
	}

	{
		auto it = stringSet.find(ktl::unicode_string_view{ L"corgi" });
		ASSERT_TRUE(it != stringSet.end(), "unable to find expected entry in set");
		ASSERT_TRUE(*it == L"corgi", "found entry didn't contain expected value: %wZ", it->data());
	}

	// find code unit character string
	{
		auto it = stringSet.find(ktl::unicode_string_view{ L"𐍈" });
		ASSERT_TRUE(it != stringSet.end(), "unable to find expected entry in set");
		ASSERT_TRUE(*it == L"𐍈", "found entry didn't contain expected value: %wZ", it->data());
	}

	ktl::unicode_string extended_code_points{ L"𐍈" };
	ASSERT_TRUE(extended_code_points.byte_size() == (sizeof(L"𐍈") - sizeof(UNICODE_NULL)), "unexpected byte count for extended code point: %llu", extended_code_points.byte_size());
	ASSERT_TRUE(extended_code_points.size() == (sizeof(L"𐍈") - sizeof(UNICODE_NULL)) / sizeof(wchar_t), "unexpected character count for extended code point: %llu", extended_code_points.size());

	// iteration & erase
	ASSERT_TRUE(stringSet.find(ktl::unicode_string_view{ L"$" }) != stringSet.end(), "unable to find expected entry in set");
	auto sizeBefore = stringSet.size();
	for (auto it = stringSet.begin(); it != stringSet.end();)
	{
		if (*it == L"$")
			it = stringSet.erase(*it);
		else
			++it;
	}
	ASSERT_TRUE(stringSet.find(ktl::unicode_string_view{ L"$" }) == stringSet.end(), "erase entry still in set");
	ASSERT_TRUE(sizeBefore == stringSet.size() + 1, "more elements than expected erased from set: %llu -> %llu", sizeBefore, stringSet.size());

	return true;
}

bool test_set_performance()
{
	// Validate that lookup performance of set is superior to vector.
	const int END_ELEMENT = 250000;
	const int FIND_VALUE = END_ELEMENT / 2;
	ktl::set<int> intSet;
	ktl::vector<int> intVector;

	ASSERT_TRUE(intSet.reserve(END_ELEMENT), "failed to reserve set capacity");
	ASSERT_TRUE(intVector.reserve(END_ELEMENT), "failed to reserve vector capacity");

	for (int i = 0; i < END_ELEMENT; ++i)
	{
		ASSERT_TRUE(intSet.insert(i), "failed to insert element %d into set", i);
		ASSERT_TRUE(intVector.push_back(i), "failed to insert element %d into vector", i);
	}

	ktl::floating_point_state fpState;

	double set_find = 0.0;
	double vector_find = 0.0;
	timer t;

	{
		t.start();
		auto it = intSet.find(FIND_VALUE);
		ASSERT_TRUE(it != intSet.end(), "Didn't find expected value %d in set", FIND_VALUE);
		t.stop();
		set_find = t.elapsed();
	}

	{
		t.start();
		auto it = ktl::find(intVector.begin(), intVector.end(), FIND_VALUE);
		ASSERT_TRUE(it != intVector.end(), "Didn't find expected value %d in vector", FIND_VALUE);
		t.stop();
		vector_find = t.elapsed();
	}

	ASSERT_TRUE(vector_find > set_find, "Vector lookup was faster than set.");

	return true;
}

bool test_set_copy()
{
	ktl::set<int> originalSet;

	for (int i = 0; i < 5; ++i)
		ASSERT_TRUE(originalSet.insert(i), "failed to insert integer to set: %d", i);

	auto copy = originalSet.copy();

	ASSERT_TRUE(copy.has_value(), "copy of original set was not successful");
	ASSERT_TRUE(copy->size() == originalSet.size(), "copy and original didn't have same size.");

	for (auto& element : originalSet)
	{
		ASSERT_TRUE(copy->find(element) != copy->end(), "unable to find element from original set in copied set");
	}

	return true;
}

bool test_set()
{
	__try
	{
		if (!test_set_of_string())
			return false;

		if (!test_set_performance())
			return false;

		if (!test_set_copy())
			return false;

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::set!\n");
	return true;
}