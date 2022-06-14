#include "test.h"

#include <optional>
#include <string>

bool test_optional()
{
	__try
	{
		static_assert(!ktl::is_convertible_v<ktl::optional<ktl::unicode_string<>>, int>);

		ktl::unicode_string str{ L"foo" };
		ktl::optional<ktl::unicode_string<>> opt;

		ASSERT_FALSE(opt, "Optional had unexpected value!");
		ASSERT_FALSE(opt.has_value(), "Optional had unexpected value!");
		opt.emplace(str);
		ASSERT_TRUE(opt, "Optional has unexpected value!");
		ASSERT_TRUE(opt.has_value(), "Optional has unexpected value!");

		ASSERT_TRUE(opt.value() == L"foo", "Unexpected optional value!");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::optional!\n");
	return true;
}