#include "test.h"

#include <string>
#include <vector>

bool test_vector_basic()
{
	__try
	{
		ktl::vector<int> vec;

		ASSERT_TRUE(vec.empty(), "vector was not empty after default construction");
		ASSERT_TRUE(vec.size() == 0, "vector size was not zero after default construction");

		for (int i = 0; i < 10; ++i)
		{
			ASSERT_TRUE(vec.push_back(i), "failed lvalue push_back");
			ASSERT_TRUE(vec[i] == i, "unexpected value in vector after push_back");
		}

		ASSERT_TRUE(vec.size() == 10, "unexpected vector size after pushing lvalues");

		int x = 5;
		for (int i = 0; i < 5; ++i)
		{
			ASSERT_TRUE(vec.push_back(ktl::move(x)), "failed rvalue push_back");
			ASSERT_TRUE(vec.back() == x, "unexpected value in vector after rvalue push_back");
		}

		ASSERT_TRUE(vec.size() == 15, "unexpected vector size after pushing rvalues");

		for (int i = 0; i < 5; ++i)
		{
			ASSERT_TRUE(vec.emplace_back(i), "failed emplace_back");
			ASSERT_TRUE(vec.back() == i, "unexpected value in vector after emplace_back");
		}

		ASSERT_TRUE(vec.size() == 20, "unexpected vector size after emplacing rvalues");
		vec.clear();
		ASSERT_TRUE(vec.size() == 0, "unexpected vector size after clearing");

		ASSERT_TRUE(vec.capacity() >= 20, "vector capacity unexpectedly low after clearing");
		auto initialCapacity = vec.capacity();
		ASSERT_TRUE(vec.reserve(initialCapacity + 1), "vector reserve failed");
		ASSERT_TRUE(vec.capacity() == initialCapacity + 1, "vector reserve had unexpected impact on capacity");
		ASSERT_TRUE(vec.size() == 0, "vector reserve affected size");
		auto newSize = vec.capacity() * 2;
		ASSERT_TRUE(vec.resize(newSize), "failed to resize vector to twice current capacity");
		ASSERT_TRUE(vec.size() == newSize, "unexpected size after vector resize");
		vec.pop_back();
		ASSERT_TRUE(vec.size() == newSize - 1, "unexpected size after vector pop_back");

		ktl::vector<ktl::unicode_string> strVec;
		ktl::unicode_string str{ L"string" };
		ASSERT_TRUE(strVec.push_back(str), "failed to push string");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("Exception thrown in test: %#x\n", GetExceptionCode());
		return false;
	}

	return true;
}