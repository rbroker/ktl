#include "ktl_core.h"
#include "ktl_crt.h"

#include <new>

#ifndef KTL_OMIT_CRT_STUB

namespace ktl
{
	/* Enable floating point marker. */
	extern "C" { int _fltused = 1; }

	/* Memory Allocations */
#if KTL_TRACK_ALLOCATIONS
	volatile INT64 ktl_pool_alloc_count__;
	volatile INT64 ktl_pool_free_count__;
#endif

	[[nodiscard]] void* pool_alloc(size_t size, pool_type type)
	{
		POOL_TYPE pool = NonPagedPoolNx;
		if (type == pool_type::Paged)
			pool = PagedPool;

#if KTL_TRACK_ALLOCATIONS
		auto p = ExAllocatePoolZero(pool, size, KTL_POOL_TAG);

		if (p != nullptr)
			InterlockedIncrement64(&ktl_pool_alloc_count__);

		return p;
#else
		// Should be replaced with ExAllocatePool2 if we can drop support for versions of Win10 older than v2004.
		// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-exallocatepool2
		return ExAllocatePoolZero(pool, size, KTL_POOL_TAG);
#endif
	}

	void pool_free(void* p)
	{
#if KTL_TRACK_ALLOCATIONS
		InterlockedIncrement64(&ktl_pool_free_count__);
#endif
		::ExFreePoolWithTag(p, KTL_POOL_TAG);
	}

	void validate_pool_allocations()
	{
#if KTL_TRACK_ALLOCATIONS
		if (ktl_pool_alloc_count__ != ktl_pool_free_count__)
			KTL_LOG_ERROR("Alloc/Free mismatch: %lld/%lld\n", ktl_pool_alloc_count__, ktl_pool_free_count__);
		else
			KTL_LOG_TRACE("pool alloc count: %lld, pool free count: %lld\n", ktl_pool_alloc_count__, ktl_pool_free_count__);
#endif
	}

	using pvfv__ = void(__cdecl*)(void);
	using pifv__ = int(__cdecl*)(void);

	/* Dynamic initializers are placed in .CRT$XCU, so we define XCA and XCZ as start/end pointers
	 *  same as the usermode CRT. That'll let us spin through the table of initializer functions
	 *  and call them all in turn.
	 */
	extern "C"
	{
		// C dynamic initialization
		#pragma section(".CRT$XIA", read)
		#pragma section(".CRT$XIZ", read)
		__declspec(allocate(".CRT$XIA")) pifv__ xi_a__[] = { nullptr };
		__declspec(allocate(".CRT$XIZ")) pifv__ xi_z__[] = { nullptr };

		// C++ dynamic initialization
		#pragma section(".CRT$XCA", read)
		#pragma section(".CRT$XCZ", read)
		__declspec(allocate(".CRT$XCA")) pvfv__ xc_a__[] = { nullptr };
		__declspec(allocate(".CRT$XCZ")) pvfv__ xc_z__[] = { nullptr };

		// C pre terminators
		#pragma section(".CRT$XPA", read)
		#pragma section(".CRT$XPZ", read)
		__declspec(allocate(".CRT$XPA")) pvfv__ xp_a__[] = { nullptr };
		__declspec(allocate(".CRT$XPZ")) pvfv__ xp_z__[] = { nullptr };

		// C terminators
		#pragma section(".CRT$XTA", read)
		#pragma section(".CRT$XTZ", read)
		__declspec(allocate(".CRT$XTA")) pvfv__ xt_a__[] = { nullptr };
		__declspec(allocate(".CRT$XTZ")) pvfv__ xt_z__[] = { nullptr };
	}

	struct __at_exit_fn_element
	{
		LIST_ENTRY Entry;
		pvfv__ AtExitCallback;
	};

	LIST_ENTRY at_exit_fn_list__;
	PKSPIN_LOCK at_exit_lock__ = nullptr;

	// https://docs.microsoft.com/en-us/cpp/c-runtime-library/crt-initialization?view=msvc-160
	// This will call every function pointer between start & end, skipping null function pointers.
	void walk_function_table(pvfv__* start, pvfv__* end)
	{
		for (auto curr = start; curr < end; ++curr)
		{
			if (!(*curr))
				continue;

			pvfv__ fn = **curr;
			fn();
		}
	}

	[[nodiscard]] int walk_function_table(pifv__* start, pifv__* end)
	{
		for (auto curr = start; curr < end; ++curr)
		{
			if (!(*curr))
				continue;

			pifv__ fn = **curr;
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
#if KTL_TRACK_ALLOCATIONS
		ktl_pool_alloc_count__ = 0;
		ktl_pool_free_count__ = 0;
#endif

		at_exit_lock__ = new_spinlock();

		if (!at_exit_lock__)
		{
			KTL_LOG_ERROR("Failed to initialize atexit spinlock\n");
			return false;
		}

		InitializeListHead(&at_exit_fn_list__);

		__try
		{
			// Call all C dynamic initializers
			if (walk_function_table(xi_a__, xi_z__))
			{
				KTL_LOG_ERROR("Failed to walk C dynamic initializers\n");
				return false;
			}

			// Call all C++ dynamic initializers.
			walk_function_table(xc_a__, xc_z__);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			KTL_LOG_ERROR("Caught structured exception, while walking dynamic initializers\n");
			return false;
		}

		return true;
	}

	int atexit(pvfv__ functionPtr)
	{
		if (!at_exit_lock__)
			return 1;

		auto atExit = static_cast<__at_exit_fn_element*>(pool_alloc(sizeof(__at_exit_fn_element), pool_type::NonPaged));

		if (!atExit)
			return 2;

		atExit->AtExitCallback = functionPtr;

		KIRQL oldIrq;
		KeAcquireSpinLock(at_exit_lock__, &oldIrq);

		InsertHeadList(&at_exit_fn_list__, &(atExit->Entry));

		KeReleaseSpinLock(at_exit_lock__, oldIrq);

		return 0;
	}

	void unload_runtime()
	{
		if (!at_exit_lock__)
		{
			KTL_LOG_ERROR("Unable to run atexit() calls due to invalid spinlock\n");
			return;
		}

		KIRQL oldIrq;
		KeAcquireSpinLock(at_exit_lock__, &oldIrq);

		while (true)
		{
			auto head = RemoveHeadList(&at_exit_fn_list__);
			if (head == &at_exit_fn_list__)
				break;

			auto entry = CONTAINING_RECORD(head, __at_exit_fn_element, Entry);
			entry->AtExitCallback();
			pool_free(entry);
		}

		KeReleaseSpinLock(at_exit_lock__, oldIrq);

		pool_free(at_exit_lock__);
		at_exit_lock__ = nullptr;

		__try
		{
			// Call all pre-terminators
			walk_function_table(xp_a__, xp_z__);

			// Call all C terminators
			walk_function_table(xt_a__, xt_z__);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}

		ktl::validate_pool_allocations();
	}
}

int __cdecl atexit(void(__cdecl* func)(void))
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
	return ktl::pool_alloc(n, pool);
}

// Placement New.
void* operator new(size_t n, void* p)
{
	UNREFERENCED_PARAMETER(n);
	return p;
}

// Global Deletes
void __cdecl operator delete(void* p)
{
	ktl::pool_free(p);
}

void __cdecl operator delete(void* p, size_t n)
{
	UNREFERENCED_PARAMETER(n);

	ktl::pool_free(p);
}

void __cdecl operator delete(void* p, std::align_val_t a)
{
	UNREFERENCED_PARAMETER(a);

	ktl::pool_free(p);
}

void __cdecl operator delete(void* p, size_t n, std::align_val_t a)
{
	UNREFERENCED_PARAMETER(n);
	UNREFERENCED_PARAMETER(a);

	ktl::pool_free(p);
}

void __cdecl operator delete[](void* p)
{
	ktl::pool_free(p);
}

void __cdecl operator delete[](void* p, size_t n)
{
	UNREFERENCED_PARAMETER(n);

	ktl::pool_free(p);
}

void __cdecl operator delete[](void* p, std::align_val_t a)
{
	UNREFERENCED_PARAMETER(a);

	ktl::pool_free(p);
}

void __cdecl operator delete[](void* p, size_t n, std::align_val_t a)
{
	UNREFERENCED_PARAMETER(n);
	UNREFERENCED_PARAMETER(a);

	ktl::pool_free(p);
}

#endif