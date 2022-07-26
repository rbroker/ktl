#include "test.h"

#include <map>

bool test_map()
{
	__try
	{
		ktl::flat_map<int, int> m;

		m.insert(0, 1);

		{
			auto it = m.find(0);
			ASSERT_TRUE(it != m.end(), "Unexpected end iterator!");
			auto [key, value] = *it;
			ASSERT_TRUE(key == 0, "Unexpected map key");
			ASSERT_TRUE(value == 1, "Unexpected map value");
		}

		auto eraseIt = m.erase(0);
		ASSERT_TRUE(eraseIt == ktl::end(m), "Unexpected next iterator after erasing last element from map.");

		// Find erased value.
		ASSERT_TRUE(m.find(0) == m.end(), "Unexpectedly found erased value!");

		// Overwrite key.
		ASSERT_TRUE(m.insert(0, 1) != m.end(), "Unexpected result of insertion.");
		ASSERT_TRUE(m.insert(0, 2) != m.end(), "Unexpected result of insertion.");

		ASSERT_TRUE(m.size() == 1, "Unexpected map size after overwriting key.");

		const size_t BIG_MAP_SIZE = 0x80000;
		ASSERT_FALSE(m.reserve(BIG_MAP_SIZE - 16), "Able to reserve non power 2 size!");
		ASSERT_TRUE(m.reserve(BIG_MAP_SIZE), "Unable to reserve map capacity!");
		for (int i = 1; i < BIG_MAP_SIZE; ++i)
		{
			m.insert(i, i + 1);
		}

		ASSERT_TRUE(m.size() == BIG_MAP_SIZE, "Unexpected number of elements in map after many insertions");
		ASSERT_TRUE(m.capacity() > m.size(), "Map growth strategy didn't leave us with excess elements");

		// Remove 1/4 of the elements in the map, this should free up enough space for
		// us to shrink the map to a smaller power of 2 (0x80000), without exceeding
		// our max load factor.
		for (int i = 0; i < BIG_MAP_SIZE / 4; ++i)
		{
			m.erase(i);
		}

		auto initialCapacity = m.capacity();
		ASSERT_TRUE(m.shrink_to_fit(), "Map minimisation failed.");
		ASSERT_TRUE(m.capacity() < initialCapacity, "Unexpected map capacity after shrinkage! (%llu >= %llu, %llu)", m.capacity(), initialCapacity, m.size());
		ASSERT_TRUE(m.capacity() > m.size(), "Unexpected map capacity after shrinkage!");

		{
			auto bigFind = m.find(BIG_MAP_SIZE - 1);
			auto [key, value] = *bigFind;
			ASSERT_TRUE(key == BIG_MAP_SIZE - 1, "Unexpected value of key of found element.");
			ASSERT_TRUE(value == key + 1, "Unexpected value of value of found element.");
		}

		ktl::flat_map<int, int> min_shrink;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::flat_map!\n");
	return true;
}