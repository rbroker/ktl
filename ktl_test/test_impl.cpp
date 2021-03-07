#include "test.h"

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

bool test_vector()
{
	__try
	{
		ktl::vector<int> vec;

		ASSERT_TRUE(vec.empty(), "vector was not empty after default construction");
		ASSERT_TRUE(vec.size() == 0, "vector size was not zero after default construction: %llu", vec.size());

		for (int i = 0; i < 10; ++i)
		{
			ASSERT_TRUE(vec.push_back(i), "failed lvalue push_back");
			ASSERT_TRUE(vec[i] == i, "unexpected value in vector after push_back: %d != %d", vec[i], i);
		}

		ASSERT_TRUE(vec.size() == 10, "unexpected vector size after pushing lvalues");

		int x = 5;
		for (int i = 0; i < 5; ++i)
		{
			ASSERT_TRUE(vec.push_back(ktl::move(x)), "failed rvalue push_back");
			ASSERT_TRUE(vec.back() == x, "unexpected value in vector after rvalue push_back: %d != %d", vec.back(), x);
		}

		ASSERT_TRUE(vec.size() == 15, "unexpected vector size after pushing rvalues");

		for (int i = 0; i < 5; ++i)
		{
			ASSERT_TRUE(vec.emplace_back(i), "failed emplace_back");
			ASSERT_TRUE(vec.back() == i, "unexpected value in vector after emplace_back: %d != %d", vec.back(), i);
		}

		ASSERT_TRUE(vec.size() == 20, "unexpected vector size after emplacing rvalues: %llu", vec.size());
		vec.clear();
		ASSERT_TRUE(vec.size() == 0, "unexpected vector size after clearing: %llu", vec.size());

		ASSERT_TRUE(vec.capacity() >= 20, "vector capacity unexpectedly low after clearing: %llu", vec.capacity());
		auto initialCapacity = vec.capacity();
		ASSERT_TRUE(vec.reserve(initialCapacity + 1), "vector reserve failed");
		ASSERT_TRUE(vec.capacity() == initialCapacity + 1, "vector reserve had unexpected impact on capacity: %llu", vec.capacity());
		ASSERT_TRUE(vec.size() == 0, "vector reserve affected size: %llu", vec.size());
		auto newSize = vec.capacity() * 2;
		ASSERT_TRUE(vec.resize(newSize), "failed to resize vector to twice current capacity");
		ASSERT_TRUE(vec.size() == newSize, "unexpected size after vector resize: %llu", vec.size());
		vec.pop_back();
		ASSERT_TRUE(vec.size() == newSize - 1, "unexpected size after vector pop_back: %llu", vec.size());

		ktl::vector<ktl::unicode_string> strVec;
		ktl::unicode_string str{ L"string" };
		ASSERT_TRUE(strVec.push_back(str), "failed to push string");

		ktl::vector<complex_object> structVec;
		ASSERT_TRUE(structVec.emplace_back(L"hello world", 10), "failed to emplace first structure in vector");
		ASSERT_TRUE(structVec.emplace_back(L"world"), "failed to emplace second structure in vector");

		ASSERT_TRUE(structVec[0].Value == 10, "invalid value in first emplaced structure");
		ASSERT_TRUE(structVec[0].Name == L"hello world", "invalid name in first emplaced structure: %wZ", structVec[0].Name.data());
		ASSERT_TRUE(structVec[1].Value == 5, "invalid value in second emplaced structure");
		ASSERT_TRUE(structVec[1].Name == L"world", "invalid name in second emplaced structure: %wZ", structVec[1].Name.data());
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
		ktl::unicode_string from_literal{ L"my_string" };
		ktl::unicode_string from_string{ from_literal };
		ktl::unicode_string default_constructed{};

		ASSERT_TRUE(from_literal.size() == 9, "string constructed from literal has unexpected length: %llu", from_literal.size());
		ASSERT_TRUE(from_string.size() == 9, "string constructed from string has unexpected length: %llu", from_string.size());
		ASSERT_TRUE(default_constructed.empty(), "default constructed string was not empty");
		ASSERT_TRUE(default_constructed.size() == 0, "default constructed string had non-zero size: %llu", default_constructed.size());

		ASSERT_TRUE(from_literal == from_string, "constructed strings did not match");
		ktl::unicode_string moved = ktl::move(from_string);

		ASSERT_TRUE(moved == from_literal, "moved string didn't match");
		ASSERT_TRUE(from_string != from_literal, "string matched after being moved away");
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