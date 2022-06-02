#pragma once

#include "common.h"
#include <kernel>
#include <string>
#include <string_view>
#include <vector>

#define KTL_TEST(Name, TestBody)												\
bool KTLTest_##Name(void)														\
{																				\
	__try                                                                       \
	##TestBody                                                                  \
	__except (EXCEPTION_EXECUTE_HANDLER)                                        \
	{                                                                           \
		LOG_ERROR("Exception thrown in test: %#x\n", GetExceptionCode());       \
		return false;                                                           \
	}                                                                           \
                                                                                \
	LOG_TRACE("[OK] " #Name "!\n");                                             \
	return true;                                                                \
}                                                                               \

#define ASSERT_TRUE(x, fmt, ...) do { if (!(##x)) { LOG_ERROR("[NG] (" #x ") " fmt "\n", __VA_ARGS__); return false; } } while(0)
#define ASSERT_FALSE(x, fmt, ...) do { if ((##x)) { LOG_ERROR("[NG] (" #x ") " fmt "\n", __VA_ARGS__); return false; } } while(0)

bool test_set();
bool test_vector();
bool test_unicode_string();
bool test_unicode_string_view();
bool test_list();
bool test_memory();
bool test_tuple();

struct timer
{
	timer() :
		start_{},
		end_{},
		frequency_{}
	{
	}

	void start()
	{
		start_ = KeQueryPerformanceCounter(&frequency_);
	}

	void stop()
	{
		end_ = KeQueryPerformanceCounter(nullptr);
	}

	double elapsed()
	{
		double end = static_cast<double>(end_.QuadPart);
		double start = static_cast<double>(start_.QuadPart);

		return (end - start) / static_cast<double>(frequency_.QuadPart);
	}

private:
	LARGE_INTEGER start_;
	LARGE_INTEGER end_;
	LARGE_INTEGER frequency_;
};

struct complex_copyable_object
{
	complex_copyable_object() = default;
	complex_copyable_object(const complex_copyable_object& other) :
		Name(other.Name),
		Value(other.Value)
	{
	}

	complex_copyable_object(ktl::unicode_string_view name, int value) :
		Name(name),
		Value(value)
	{
	}

	complex_copyable_object(ktl::unicode_string_view name) :
		Name(name)
	{
	}

	ktl::unicode_string<> Name;
	int Value = 5;
	ktl::vector<int> Vec;

private:
	int NonStandard = -5;
};

struct complex_object
{
	complex_object() = default;
	complex_object(complex_object&&) = default;
	complex_object& operator=(complex_object&&) = default;

	complex_object(ktl::unicode_string_view name, int value) :
		Name(name),
		Value(value)
	{
	}

	complex_object(ktl::unicode_string_view name) :
		Name(name)
	{
	}

	ktl::unicode_string<> Name;
	int Value = 5;
	ktl::vector<int> Vec;

private:
	int NonStandard = -5;
};
