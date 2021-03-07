#include "test.h"

#include <set>
#include <string>
#include <vector>

struct complex_object
{
	complex_object() = default;

	complex_object(ktl::unicode_string_view name, int value) :
		Name(name),
		Value(value)
	{
	}

	complex_object(ktl::unicode_string_view name) :
		Name(name)
	{
	}

	ktl::unicode_string Name;
	int Value = 5;
	ktl::vector<int> Vec;
};

bool test_set()
{
	__try
	{
		ktl::set<ktl::unicode_string> set;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::set!\n");
	return true;
}

bool test_vector()
{
	__try
	{
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
		ktl::vector<ktl::unicode_string> strVec;
		ktl::unicode_string str{ L"string" };
		ASSERT_TRUE(strVec.push_back(str), "failed to push string");
		ASSERT_TRUE(strVec.emplace_back(L"other"), "failed to emplace string");
		ASSERT_TRUE(strVec.size() == 2, "incorrect size after pushing strings");
		ASSERT_TRUE(strVec[0] == L"string", "invalid value in vector after pushing string");
		ASSERT_TRUE(strVec[1] == L"other", "invalid value in vector after pushing string");

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
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::vector!\n");
	return true;
}

bool test_unicode_string()
{
	__try
	{
		// Default & copy constructors
		ktl::unicode_string from_literal{ L"my_string" };
		ktl::unicode_string from_string{ from_literal };
		ktl::unicode_string default_constructed{};

		ASSERT_TRUE(from_literal.size() == 9, "string constructed from literal has unexpected length: %llu", from_literal.size());
		ASSERT_TRUE(from_string.size() == 9, "string constructed from string has unexpected length: %llu", from_string.size());
		ASSERT_TRUE(default_constructed.empty(), "default constructed string was not empty");
		ASSERT_TRUE(default_constructed.size() == 0, "default constructed string had non-zero size: %llu", default_constructed.size());

		ASSERT_TRUE(from_literal == from_string, "constructed strings did not match");

		// construct, with length
		ktl::unicode_string literal2{ L"my_string", 2 };
		ASSERT_TRUE(literal2 == L"my", "substring constructor didn't initialize to expected value: %wZ", literal2.data());

		// move construct
		ktl::unicode_string moveConstructed{ ktl::move(from_literal) };
		ASSERT_TRUE(moveConstructed == L"my_string", "move constructed string didn't match");
		ASSERT_TRUE(from_literal != L"my_string", "string matched after being move constructed away");

		// move assign
		ktl::unicode_string moveAssigned = ktl::move(from_string);
		ASSERT_TRUE(moveAssigned == L"my_string", "move assigned string didn't match");
		ASSERT_TRUE(from_string != L"my_string", "string matched after being move assigned away");

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::unicode_string!\n");
	return true;
}

bool test_unicode_string_view()
{
	__try
	{
		constexpr ktl::unicode_string_view compile_time{ L"compile_time" };
		constexpr ktl::unicode_string_view other_compile_time{ L"other_compile_time" };

		ASSERT_TRUE(compile_time != other_compile_time, "compile time strings matched");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::unicode_string_view!\n");
	return true;
}