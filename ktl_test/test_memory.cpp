#include "test.h"

#include <memory>

bool test_memory()
{
	__try
	{
		{
			ktl::unique_ptr<int> simple_ptr = ktl::make_unique<int>(ktl::pool_type::NonPaged, 5);
			ASSERT_TRUE(simple_ptr, "unexpectedly failed to allocate unique_ptr");
			ASSERT_TRUE((*simple_ptr) == 5, "unique_ptr value not what was expected");

			auto raw_ptr = simple_ptr.get();
			ASSERT_FALSE(raw_ptr == nullptr, "unexpected null pointer");

			ktl::unique_ptr<int> smart_ptr = simple_ptr.release();
			ASSERT_TRUE(smart_ptr.get() == raw_ptr, "unexpected value after releasing ptr");
			ASSERT_TRUE(simple_ptr.get() == nullptr, "unexpected value after releasing ptr");

			auto smart2 = ktl::move(smart_ptr);
			ASSERT_TRUE(raw_ptr == smart2.get(), "unexpected value after move construction");

			smart2.reset();
			ASSERT_TRUE(smart2.get() == nullptr, "unique_ptr still had a value after resetting");
		}

		{
			auto complex_ptr = ktl::make_unique<complex_object[]>(ktl::pool_type::NonPaged, 6);
			for (int i = 0; i < 6; ++i)
			{
				ASSERT_TRUE(complex_ptr[i].Value == 5, "unexpected default value for complex object");
				ASSERT_TRUE(complex_ptr[i].Name == L"", "unexpected default value for complex object");
			}

			ASSERT_TRUE(complex_ptr->Value == 5, "unexpected value in first element of array");

			auto raw_ptr = complex_ptr.get();
			ASSERT_FALSE(raw_ptr == nullptr, "unexpected null pointer for array start");

			ktl::unique_ptr<complex_object[]> smart_ptr = complex_ptr.release();
			ASSERT_TRUE(smart_ptr.get() == raw_ptr, "unexpected value after releasing ptr");
			ASSERT_TRUE(complex_ptr.get() == nullptr, "unexpected value after releasing ptr");

			auto smart2 = ktl::move(smart_ptr);
			ASSERT_TRUE(raw_ptr == smart2.get(), "unexpected value after move construction");

			smart2.reset();
			ASSERT_TRUE(smart2.get() == nullptr, "unique_ptr still had a value after resetting");
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_ERROR("[NG]: %#x\n", GetExceptionCode());
		return false;
	}

	LOG_TRACE("[OK] ktl::memory!\n");
	return true;
}