#pragma once

#include "common.h"

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
