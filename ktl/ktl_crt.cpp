#include "ktl_core.h"
#include "ktl_crt.h"

#include <new>

#ifndef KTL_OMIT_CRT_STUB

namespace ktl
{
	/* Enable floating point marker. */
	extern "C" { int _fltused = 1; }

	/* Memory Allocations */
	[[nodiscard]] void* pool_alloc(size_t size, pool_type type)
	{
		POOL_TYPE pool = NonPagedPoolNx;
		if (type == pool_type::Paged)
			pool = PagedPool;

		// Should be replaced with ExAllocatePool2 if we can drop support for versions of Win10 older than v2004.
		// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-exallocatepool2
		return ExAllocatePoolZero(pool, size, KTL_POOL_TAG);
	}

	void pool_free(void* p)
	{
		::ExFreePoolWithTag(p, KTL_POOL_TAG);
	}

	using __pvfv = void(__cdecl*)(void);
	using __pifv = int(__cdecl*)(void);

	/* Dynamic initializers are placed in .CRT$XCU, so we define XCA and XCZ as start/end_ pointers
	 *  same as the usermode CRT. That'll let us spin through the table of initializer functions
	 *  and call them all in turn.
	 */
	extern "C"
	{
		// C dynamic initialization
		#pragma section(".CRT$XIA", read)
		#pragma section(".CRT$XIZ", read)
		__declspec(allocate(".CRT$XIA")) __pifv __xi_a[] = { nullptr };
		__declspec(allocate(".CRT$XIZ")) __pifv __xi_z[] = { nullptr };

		// C++ dynamic initialization
		#pragma section(".CRT$XCA", read)
		#pragma section(".CRT$XCZ", read)
		__declspec(allocate(".CRT$XCA")) __pvfv __xc_a[] = { nullptr };
		__declspec(allocate(".CRT$XCZ")) __pvfv __xc_z[] = { nullptr };

		// C pre terminators
		#pragma section(".CRT$XPA", read)
		#pragma section(".CRT$XPZ", read)
		__declspec(allocate(".CRT$XPA")) __pvfv __xp_a[] = { nullptr };
		__declspec(allocate(".CRT$XPZ")) __pvfv __xp_z[] = { nullptr };

		// C terminators
		#pragma section(".CRT$XTA", read)
		#pragma section(".CRT$XTZ", read)
		__declspec(allocate(".CRT$XTA")) __pvfv __xt_a[] = { nullptr };
		__declspec(allocate(".CRT$XTZ")) __pvfv __xt_z[] = { nullptr };
	}

	struct __at_exit_fn_element
	{
		LIST_ENTRY Entry;
		__pvfv AtExitCallback;
	};

	LIST_ENTRY __at_exit_fn_list;
	PKSPIN_LOCK __at_exit_lock = nullptr;

	// https://docs.microsoft.com/en-us/cpp/c-runtime-library/crt-initialization?view=msvc-160
	// This will call every function pointer between start & end_, skipping null function pointers.
	void walk_function_table(__pvfv* start, __pvfv* end)
	{
		for (auto curr = start; curr < end; ++curr)
		{
			KTL_LOG_TRACE("pvfv: %p\n", *curr);

			if (!(*curr))
				continue;

			__pvfv fn = **curr;
			fn();
		}
	}

	[[nodiscard]] int walk_function_table(__pifv* start, __pifv* end)
	{
		for (auto curr = start; curr < end; ++curr)
		{
			KTL_LOG_TRACE("pifv: %p\n", *curr);

			if (!(*curr))
				continue;

			__pifv fn = **curr;
			if (fn())
				return 1;
		}

		return 0;
	}

	[[nodiscard]] PKSPIN_LOCK new_spinlock()
	{
		auto spinlock = static_cast<PKSPIN_LOCK>(pool_alloc(sizeof(KSPIN_LOCK), pool_type::NonPaged));
		if (!spinlock)
			return nullptr;

		KeInitializeSpinLock(spinlock);

		return spinlock;
	}

	[[nodiscard]] bool initialize_runtime()
	{
		__at_exit_lock = new_spinlock();

		if (!__at_exit_lock)
		{
			KTL_LOG_ERROR("Failed to initialize atexit spinlock\n");
			return false;
		}

		InitializeListHead(&__at_exit_fn_list);

		__try
		{
			// Call all C dynamic initializers
			if (walk_function_table(__xi_a, __xi_z))
			{
				KTL_LOG_ERROR("Failed to walk C dynamic initializers\n");
				return false;
			}

			KTL_LOG_TRACE("C++ initializers: %p -> %p\n", __xc_a, __xc_z);

			// Call all C++ dynamic initializers.
			walk_function_table(__xc_a, __xc_z);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			KTL_LOG_ERROR("Caught structured exception, while walking dynamic initializers\n");
			return false;
		}

		KTL_LOG_TRACE("dynamic language initialization successful\n");
		return true;
	}

	int atexit(__pvfv functionPtr)
	{
		if (!__at_exit_lock)
			return 1;

		auto atExit = static_cast<__at_exit_fn_element*>(pool_alloc(sizeof(__at_exit_fn_element), pool_type::NonPaged));

		if (!atExit)
			return 2;

		atExit->AtExitCallback = functionPtr;

		KIRQL oldIrq;
		KeAcquireSpinLock(__at_exit_lock, &oldIrq);

		InsertHeadList(&__at_exit_fn_list, &(atExit->Entry));

		KeReleaseSpinLock(__at_exit_lock, oldIrq);

		return 0;
	}

	void unload_runtime()
	{
		if (!__at_exit_lock)
		{
			KTL_LOG_ERROR("Unable to run atexit() calls due to invalid spinlock\n");
			return;
		}

		KIRQL oldIrq;
		KeAcquireSpinLock(__at_exit_lock, &oldIrq);

		while (true)
		{
			auto head = RemoveHeadList(&__at_exit_fn_list);
			if (head == &__at_exit_fn_list)
				break;

			auto entry = CONTAINING_RECORD(head, __at_exit_fn_element, Entry);

			KTL_LOG_TRACE("Calling atexit() fn: %p\n", entry->AtExitCallback);
			entry->AtExitCallback();
			pool_free(entry);
		}

		KeReleaseSpinLock(__at_exit_lock, oldIrq);

		pool_free(__at_exit_lock);
		__at_exit_lock = nullptr;

		KTL_LOG_TRACE("Walking function tables for C terminators...\n");

		__try
		{
			// Call all pre-terminators
			walk_function_table(__xp_a, __xp_z);

			// Call all C terminators
			walk_function_table(__xt_a, __xt_z);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}

		KTL_LOG_TRACE("Successfully unloaded runtime.\n");
	}
}

int atexit(void(__cdecl* func)(void))
{
	return ktl::atexit(func);
}

// Global Pool New
void* operator new(size_t n, ktl::pool_type pool)
{
	return ktl::pool_alloc(n, pool);
}

// Global Pool Array New
void* operator new[](size_t n, ktl::pool_type pool)
{
	return ktl::pool_alloc(n, pool); ;
}

// Placement New.
void* operator new(size_t n, void* p)
{
	UNREFERENCED_PARAMETER(n);
	return p;
}

// Global Deletes
void operator delete(void* p)
{
	ktl::pool_free(p);
}

void operator delete(void* p, size_t n)
{
	UNREFERENCED_PARAMETER(n);

	ktl::pool_free(p);
}

void operator delete[](void* p)
{
	ktl::pool_free(p);
}

void operator delete[](void* p, size_t n)
{
	UNREFERENCED_PARAMETER(n);

	ktl::pool_free(p);
}

#endif