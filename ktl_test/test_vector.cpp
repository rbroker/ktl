#include "test.h"

bool test_vector_copy()
{
	ktl::vector<int> originalVector;

	for (int i = 0; i < 5; ++i)
		ASSERT_TRUE(originalVector.push_back(i), "failed to insert integer to vector: %d", i);

	auto copy = originalVector.copy();

	ASSERT_TRUE(copy.has_value(), "copy of original vector was not successful");
	ASSERT_TRUE(copy->size() == originalVector.size(), "copy and original didn't have same size.");

	for (size_t i = 0; i < originalVector.size(); ++i)
	{
		ASSERT_TRUE((*copy)[i] == originalVector[i], "unable to find element from original vector in copied vector");
	}

	return true;
}

bool test_vector()
{
	__try
	{
		if (!test_vector_copy())
			return false;

		// default constructor
		ktl::vector<int> vec;

		ASSERT_TRUE(vec.empty(), "vector was not empty after default construction");
		ASSERT_TRUE(vec.size() == 0, "vector size was not zero after default construction: %llu", vec.size());
		ASSERT_TRUE(vec.capacity() < 10, "vector capacity unexpected high at start of test: %llu", vec.capacity());

		// lvalue push
		for (int i = 0; i < 10; ++i)
		{
			ASSERT_TRUE(vec.push_back(i), "failed lvalue push_back");
		}

		for (int i = 0; i < 10; ++i)
		{
			ASSERT_TRUE(vec[i] == i, "unexpected value in vector after push_back/resize: %d != %d", vec[i], i);
		}

		ASSERT_TRUE(vec.size() == 10, "unexpected vector size after pushing lvalues");

		// rvalue push
		int x = 5;
		for (int i = 0; i < 5; ++i)
		{
			ASSERT_TRUE(vec.push_back(ktl::move(x)), "failed rvalue push_back");
			ASSERT_TRUE(vec.back() == x, "unexpected value in vector after rvalue push_back: %d != %d", vec.back(), x);
		}
		ASSERT_TRUE(vec.size() == 15, "unexpected vector size after pushing rvalues");

		// emplace back
		for (int i = 0; i < 5; ++i)
		{
			ASSERT_TRUE(vec.emplace_back(i), "failed emplace_back");
			ASSERT_TRUE(vec.back() == i, "unexpected value in vector after emplace_back: %d != %d", vec.back(), i);
		}
		ASSERT_TRUE(vec.size() == 20, "unexpected vector size after emplacing rvalues: %llu", vec.size());

		// clear
		vec.clear();
		ASSERT_TRUE(vec.size() == 0, "unexpected vector size after clearing: %llu", vec.size());
		ASSERT_TRUE(vec.capacity() >= 20, "vector capacity unexpectedly low after clearing: %llu", vec.capacity());

		// reserve
		auto initialCapacity = vec.capacity();
		ASSERT_TRUE(vec.reserve(initialCapacity + 1), "vector reserve failed");
		ASSERT_TRUE(vec.capacity() == initialCapacity + 1, "vector reserve had unexpected impact on capacity: %llu", vec.capacity());
		ASSERT_TRUE(vec.size() == 0, "vector reserve affected size: %llu", vec.size());

		// resize
		auto newSize = vec.capacity() * 2;
		ASSERT_TRUE(vec.resize(newSize), "failed to resize vector to twice current capacity");
		ASSERT_TRUE(vec.size() == newSize, "unexpected size after vector resize: %llu", vec.size());

		// pop_back
		vec.pop_back();
		ASSERT_TRUE(vec.size() == newSize - 1, "unexpected size after vector pop_back: %llu", vec.size());

		// non-trivial types
		ktl::vector<ktl::unicode_string<>> strVec;
		ktl::unicode_string str{ L"string" };
		ASSERT_TRUE(strVec.push_back(str), "failed to push string");
		ASSERT_TRUE(strVec.emplace_back(L"other"), "failed to emplace string");
		ASSERT_TRUE(strVec.size() == 2, "incorrect size after pushing strings");
		ASSERT_TRUE(strVec[0] == L"string", "invalid value in vector after pushing string");
		ASSERT_TRUE(strVec[1] == L"other", "invalid value in vector after pushing string");

		static_assert(!ktl::is_trivially_copyable_v<complex_object>);
		static_assert(!ktl::is_standard_layout_v<complex_object>);

		ktl::vector<complex_object> structVec;
		ASSERT_TRUE(structVec.emplace_back(L"hello world", 10), "failed to emplace first structure in vector");
		ASSERT_TRUE(structVec.emplace_back(L"world"), "failed to emplace second structure in vector");

		ASSERT_TRUE(structVec[0].Value == 10, "invalid value in first emplaced structure");
		ASSERT_TRUE(structVec[0].Name == L"hello world", "invalid name in first emplaced structure: %wZ", structVec[0].Name.data());
		ASSERT_TRUE(structVec[1].Value == 5, "invalid value in second emplaced structure");
		ASSERT_TRUE(structVec[1].Name == L"world", "invalid name in second emplaced structure: %wZ", structVec[1].Name.data());

		// range-based for
		size_t count = 0;
		for (const auto& s : strVec)
		{
			switch (count++)
			{
				case 0:
					ASSERT_TRUE(s == L"string", "first element in iteration was incorrect");
					break;
				case 1:
					ASSERT_TRUE(s == L"other", "second element in iteration was incorrect");
					break;
			}
		}
		ASSERT_TRUE(count == 2, "unexpected number of elements in iteration");

		// erase
		vec.clear();
		for (int i = 0; i < 5; ++i)
			ASSERT_TRUE(vec.push_back(i), "failed to push back elements");

		// erase front
		for (auto it = vec.begin(); it != vec.end(); )
		{
			if (*it == 0)
				it = vec.erase(it);

			++it;
		}

		ASSERT_TRUE(vec.size() == 4, "unexpected size after erase: %llu", vec.size());
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 0; }) == vec.end(), "found erased element");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 1; }) != vec.end(), "didn't find expected element (1)");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 2; }) != vec.end(), "didn't find expected element (2)");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 3; }) != vec.end(), "didn't find expected element (3)");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 4; }) != vec.end(), "didn't find expected element (4)");

		// erase back
		for (auto it = vec.begin(); it != vec.end(); )
		{
			if (*it == 4)
				it = vec.erase(it);

			++it;
		}

		ASSERT_TRUE(vec.size() == 3, "unexpected size after erase: %llu", vec.size());
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 1; }) != vec.end(), "didn't find expected element (1)");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 2; }) != vec.end(), "didn't find expected element (2)");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 3; }) != vec.end(), "didn't find expected element (3)");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 4; }) == vec.end(), "found erased element");

		// erase middle
		for (auto it = vec.begin(); it != vec.end(); )
		{
			if (*it == 2)
				it = vec.erase(it);

			++it;
		}

		ASSERT_TRUE(vec.size() == 2, "unexpected size after erase: %llu", vec.size());
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 1; }) != vec.end(), "didn't find expected element (1)");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 2; }) == vec.end(), "found erased element");
		ASSERT_TRUE(ktl::find_if(vec.begin(), vec.end(), [](auto e) -> bool { return e == 3; }) != vec.end(), "didn't find expected element (3)");

		// Erase remaining elements
		for (auto it = vec.begin(); it != vec.end();)
			it = vec.erase(it);

		ASSERT_TRUE(vec.empty(), "vector not empty after erasing all elements");

		// erase complex
		ktl::vector<ktl::unicode_string<>> vecStr;
		static_assert(!ktl::is_trivially_copyable_v<ktl::unicode_string<>>);

		ASSERT_TRUE(vecStr.emplace_back(L"one"), "failed to emplace test string");
		ASSERT_TRUE(vecStr.emplace_back(L"two"), "failed to emplace test string");
		ASSERT_TRUE(vecStr.emplace_back(L"three"), "failed to emplace test string");
		ASSERT_TRUE(vecStr.emplace_back(L"four"), "failed to emplace test string");
		ASSERT_TRUE(vecStr.emplace_back(L"five"), "failed to emplace test string");

		// erase complex front
		for (auto it = vecStr.begin(); it != vecStr.end(); )
		{
			if (*it == L"one")
				it = vecStr.erase(it);

			++it;
		}

		ASSERT_TRUE(vecStr.size() == 4, "unexpected size after erase: %llu", vecStr.size());
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"one"; }) == vecStr.end(), "found erased element");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"two"; }) != vecStr.end(), "didn't find expected element (two)");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"three"; }) != vecStr.end(), "didn't find expected element (three)");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"four"; }) != vecStr.end(), "didn't find expected element (four)");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"five"; }) != vecStr.end(), "didn't find expected element (five)");

		// erase back
		for (auto it = vecStr.begin(); it != vecStr.end(); )
		{
			if (*it == L"five")
				it = vecStr.erase(it);

			++it;
		}

		ASSERT_TRUE(vecStr.size() == 3, "unexpected size after erase: %llu", vecStr.size());
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"two"; }) != vecStr.end(), "didn't find expected element (two)");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"three"; }) != vecStr.end(), "didn't find expected element (three)");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"four"; }) != vecStr.end(), "didn't find expected element (four)");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"five"; }) == vecStr.end(), "found erased element");

		// erase middle
		for (auto it = vecStr.begin(); it != vecStr.end(); )
		{
			if (*it == L"three")
				it = vecStr.erase(it);

			++it;
		}

		ASSERT_TRUE(vecStr.size() == 2, "unexpected size after erase: %llu", vecStr.size());
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"two"; }) != vecStr.end(), "didn't find expected element (two)");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"three"; }) == vecStr.end(), "found erased element");
		ASSERT_TRUE(ktl::find_if(vecStr.begin(), vecStr.end(), [](auto e) -> bool { return e == L"four"; }) != vecStr.end(), "didn't find expected element (four)");

		// Erase remaining elements
		for (auto it = vecStr.begin(); it != vecStr.end();)
			it = vecStr.erase(it);

		ASSERT_TRUE(vecStr.empty(), "vector not empty after erasing all elements");

		// huge vector.
		vec.clear();

		for (int i = 0; i < 1000000; ++i)
			ASSERT_TRUE(vec.emplace_back(i), "large emplace failed at: %d", i);

		for (int i = 0; i < 1000000; ++i)
			ASSERT_TRUE(vec[i] == i, "unexpected value at %d: %d", i, vec[i]);

		ASSERT_TRUE(vec.capacity() >= 1000000, "unexpectedly low vector capacity: %llu", vec.capacity());
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::vector!\n");
	return true;
}