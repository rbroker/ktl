#include "test.h"

#include <map>

bool test_map()
{
	__try
	{
		ktl::flat_map<int, int> m;

		m.insert(0, 1);

		auto it = m.find(0);
		ASSERT_TRUE(it != m.end(), "Unexpected end iterator!");
		auto [key, value] = *it;
		ASSERT_TRUE(key == 0, "Unexpected map key");
		ASSERT_TRUE(value == 1, "Unexpected map value");

		m.erase(0);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::flat_map!\n");
	return true;
}