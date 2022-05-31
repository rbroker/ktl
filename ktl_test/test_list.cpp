#include "test.h"

#include <list>

bool test_list_copy()
{
	ktl::list<int> originalList;

	for (int i = 0; i < 5; ++i)
		ASSERT_TRUE(originalList.push_back(i), "failed to insert integer to vlistector: %d", i);

	auto copy = originalList.copy();

	ASSERT_TRUE(copy.has_value(), "copy of original list was not successful");
	ASSERT_TRUE(copy->size() == originalList.size(), "copy and original didn't have same size.");

	for (auto& element : originalList)
	{
		ASSERT_TRUE(ktl::find(copy->begin(), copy->end(), element) != copy->end(), "unable to find element from original list in copied list");
	}

	return true;
}

bool test_list()
{
	__try
	{
		if (!test_list_copy())
			return false;

		ktl::list<int> int_list;

		ASSERT_TRUE(int_list.empty(), "default constructed list was not empty");

		// push back
		{
			for (int i = 0; i < 500; ++i)
				ASSERT_TRUE(int_list.push_back(i), "failed to push element to back of list");

			{
				auto it = int_list.begin();
				for (int i = 0; i < 500; ++i, ++it)
					ASSERT_TRUE(*it == i, "unexpected value when pushing to back of list: %d != %d", *it, i);
			}

			ASSERT_TRUE(int_list.size() == 500, "unexpected size of list after pushing elements");
			ASSERT_TRUE(int_list.front() == 0, "unexpected value at front of list");
			ASSERT_TRUE(int_list.back() == 499, "unexpected value at back of list");

			int_list.pop_front();
			ASSERT_TRUE(int_list.front() == 1, "unexpected value at front of list");

			int_list.pop_back();
			ASSERT_TRUE(int_list.back() == 498, "unexpected value at back of list");

			auto b = int_list.begin();
			auto e = int_list.end();

			for (auto it = b; it != e; ++it)
			{
				if (*it != 250 && *it != 498)
					continue;

				it = int_list.erase(it);
			}

			ASSERT_TRUE(ktl::find_if(ktl::begin(int_list), ktl::end(int_list), [](auto& v) { return v == 250; }) == int_list.end(), "found erased element in list: 250");
			ASSERT_TRUE(ktl::find_if(int_list.begin(), int_list.end(), [](auto& v) { return v == 498; }) == int_list.end(), "found erased element in list: 499");
			ASSERT_FALSE(ktl::find_if(int_list.begin(), int_list.end(), [](auto& v) { return v == 251; }) == int_list.end(), "did not find expected element in list: 251");
			ASSERT_FALSE(ktl::find_if(int_list.begin(), int_list.end(), [](auto& v) { return v == 249; }) == int_list.end(), "did not find expected element in list: 249");

			int_list.clear();
			ASSERT_TRUE(int_list.empty(), "list not empty after clearing");
			ASSERT_TRUE(int_list.size() == 0, "list empty, but has non-zero size");
		}


		// push front
		{
			for (int i = 0; i < 5; ++i)
				ASSERT_TRUE(int_list.push_front(i), "failed to push element to front of list");

			auto it = int_list.begin();
			for (int i = 4; i >= 0; --i, ++it)
				ASSERT_TRUE(*it == i, "unexpected value when pushing to front of list: %d != %d", *it, i);

			int_list.clear();
		}

		// emplace back
		{
			for (int i = 0; i < 5; ++i)
				ASSERT_TRUE(int_list.emplace_back(i), "failed to emplace element to back of list");


			auto it = int_list.begin();
			for (int i = 0; i < 5; ++i, ++it)
				ASSERT_TRUE(*it == i, "unexpected value when emplacing to front of list: %d != %d", *it, i);


			int_list.clear();
		}

		// emplace front
		{
			for (int i = 0; i < 5; ++i)
				ASSERT_TRUE(int_list.emplace_front(i), "failed to emplace element to front of list");

			auto it = int_list.begin();
			for (int i = 4; i >= 0; --i, ++it)
				ASSERT_TRUE(*it == i, "unexpected value when emplacing to front of list: %d != %d", *it, i);

			int_list.clear();
		}

		ktl::list<complex_object> complex_list;
		for (int i = 0; i < 5; ++i)
		{
			complex_object obj;
			ASSERT_TRUE(complex_list.emplace_back(ktl::move(obj)), "emplace back failed for list");
		}

		ASSERT_TRUE(complex_list.size() == 5, "unexpected list of complex objects");
		complex_list.clear();
		ASSERT_TRUE(complex_list.empty(), "list was not empty after clearing");

		ktl::list<complex_copyable_object> copyable_complex_list;
		for (int i = 0; i < 6; ++i)
		{
			complex_copyable_object obj;
			ASSERT_TRUE(copyable_complex_list.push_front(obj), "push front failed for list");
		}

		ASSERT_TRUE(copyable_complex_list.size() == 6, "unexpected list of complex objects");
		copyable_complex_list.clear();
		ASSERT_TRUE(copyable_complex_list.empty(), "list was not empty after clearing");

		ktl::list<int, ktl::nonpaged_pool_allocator> nonpaged_list;
		ASSERT_TRUE(nonpaged_list.push_back(0), "list push with custom allocator failed");

		ktl::paged_lookaisde_list<int> paged_lookaside_list{ };
		ASSERT_TRUE(paged_lookaside_list.emplace_back(1), "list emplace with custom allocator failed");
		ASSERT_TRUE(paged_lookaside_list.emplace_back(2), "list emplace with custom allocator failed");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::list!\n");
	return true;
}