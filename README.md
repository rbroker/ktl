# ktl - Kernel-mode Template Library
A tiny C++ Template Library for Windows Kernel-Mode. Not fit for production use, and only includes subset of STL-ish behaviour I find useful in personal projects.

## Stub CRT
In order to ensure proper handling of global C++ data, new, delete, etc. a partial CRT stub is included. Defining `KTL_OMIT_CRT_STUB` in the preprocessor will avoid this, but KTL still depends on a definition for all new/delete methods to be provided, and for `_fltused` to be defined if any floating point operations are in use (e.g. `set`).

`/ignore:4210` should be ignored in the linker options, as we implement the static initializers & terminators: "warning LNK4210: .CRT section exists; there may be unhandled static initializers or terminators".

The runtime must be manually initialized at the very start of `DriverEntry`, to ensure all global data is correctly initialized:

```C++
#include <ktl_core.h>

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    NTSTATUS status;

    if (!ktl::initialize_runtime())
        return CUSTOM_STATUS_KTL_INITIALIZATION_FAILED;
    ...
}
```

Similarly, we must manually unload the runtime immediately before unloading the driver:

```C++
VOID
DriverUnload(
    _In_ WDFDRIVER DriverObject
)
{
    ...

    // Unload the KTL runtime (run dynamic atexit, etc).
    ktl::unload_runtime();    
}
```

## Features
|Header|Feature|Note|
|--|--|---|
| [algorithm](ktl/algorithm) | `find`, `find_if`, `equal_to`, `min`, `max` | |
| [cstddef](ktl/cstddef) | `nullptr_t` | |
| [cstdint](ktl/cstdint) | `int8_t` -> `uint64_t` | |
| [kernel](ktl/kernel) | `floating_point_state`, `auto_irp`, `safe_user_buffer`, `object_attributes` | `ktl::floating_point_state` is needed for using [x87 floating point](https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-kesaveextendedprocessorstate).
| [limits](ktl/limits) | `<T>min`, `<T>max` | For your typical fixed-width integer types in cstdint |
| [list](ktl/list) | `list<T>` | Based on kernel [LIST_ENTRY](https://docs.microsoft.com/en-us/windows/win32/api/ntdef/ns-ntdef-list_entry) |
| [map](ktl/map) | `flat_map<K, V>` | Flat hash map implementation. |
| [memory](ktl/memory) | `addressof`, `unique_ptr<T>`, `observer_ptr<T>`, `make_unique<T>`, `paged_pool_allocator`, `nonpaged_pool_allocator`, `paged_lookaside_allocator`, `nonpaged_lookaside_allocator` | |
| [mutex](ktl/mutex) | `scoped_lock`, `mutex` | Non deadlock-avoiding lock, based on [FAST_MUTEX](https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/eprocess) |
| [optional](ktl/optional) | `optional<T>` | Partial optional implementation |
| [new](ktl/new) | `new`, `delete`, `new[]`, `delete[]`, placement `new` | You must use either placement new, or operator new overloaded with `ktl::pool_type`. All news are non-throwing. |
| [set](ktl/set) | `unordered_set<T>` | set implementation. |
| [shared_mutex](ktl/shared_mutex) | `shared_lock`, `shared_mutex` | reader-writer locking based on [ERESOURCE](https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/introduction-to-eresource-routines) |
| [string](ktl/string) | `unicode_string` | No `string` or `wstring`, everything is UTF-16 [UNICODE_STRING](https://docs.microsoft.com/en-us/windows/win32/api/ntdef/ns-ntdef-_unicode_string). |
| [string_view](ktl/string_view) | `unicode_string_view` | For the performance-conscious [UNICODE_STRING](https://docs.microsoft.com/en-us/windows/win32/api/ntdef/ns-ntdef-_unicode_string) user. |
| [tuple](ktl/tuple) | `tuple` | Minimal tuple implementation |
| [type_traits](ktl/type_traits) | `is_trivially_copyable_v`, `is_standard_layout_v` | Just enough for built-in features! |
| [utility](ktl/utility) | `scope_exit` | |
| [vector](ktl/vector) | `vector<T>` | Fan favourite, probably far from optimised. |
| [wdf](ktl/wdf) | | Various WDF helper classes |

## ktl-ctl
ktl-ctl.exe supports the usermode driver controls:
- `ktl-ctl install` : Install the driver, and create a service to start/stop it.
- `ktl-ctl uninstall` : Uninstall the driver and delete the service.
- `ktl-ctl start` : Start the installed driver service
- `ktl-ctl stop` : Stop the installed driver service
- `ktl-ctl test` : Run the unit tests

## STL?
I've abused the STL header names, but this is not an STL reimplementation, and even the bits that look similar aren't intended to be remotely standards compliant. There's a number of change-points from a typical STL implementation to account for operating in kernel-mode, and without C++ exceptions. Some things possibly worth bearing in mind:

- The `new` header defines an overload of `operator new` which accepts a `ktl::pool_type` parameter. This allows allocations to come from either the paged, or non-paged pools.
    - Scalar and array versions of `make_unique` also support specifying the pool type.
```C++
static ktl::unique_ptr<GlobalState> g_state = ktl::make_unique<GlobalState>(ktl::pool_type::NonPaged);
static int g_leaky = new (ktl::pool_type::Paged) { 0 };
```

- Operations which require allocations may fail, but not throw an exception. Calls such as `push_back` return a boolean to indicate whether the operation succeeded or failed. Return value checking in such cases is enforced by `[[nodiscard]]`.
```C++
NTSTATUS add_int()
{
    ktl::list<int> my_list;
    if (!my_list.push_back(5))    
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}
```

- Operations which would typically have returned a reference to something which required allocation (such as `emplace_back`) will return a `ktl::observer_ptr<T>` instead. This returned pointer may be `nullptr` indicating an allocation failure.
    - `ktl::observer_ptr` may be used where a `std::optional` might normally be used.
```C++
NTSTATUS add_int()
{
    ktl::list<int> my_list;
    ktl::observer_ptr<int> ptr_to_int = my_list.emplace_back(5);
    if (!ptr_to_int.has_value())
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }    

    return do_something(*ptr_to_int);
}
```

- `ktl::unicode_string` and `ktl::unicode_string_view` support most size-related methods (resize, reserve, capacity, etc) by character counts, and a `byte_` variant. 
    - The unicode here matches what the kernel is using (wchar_t).
    - The size in bytes does not include the null terminator. Resizing / requesting capacity will always allocate enough space for NULL termination.
```C++
void sizes()
{
    ktl::unicode_string str{ L"foo" };

    size_t size = str.size(); // returns "3"
    size_t bytes = str.byte_size(); // returns "6"
}
```

- KTL uses its own (probably quite terrible) allocator solution. All the supported container types support the provided allocators.
```C++
ktl::unicode_string paged_str { L"foo", };
ktl::unicode_string<ktl::nonpaged_pool_allocator> nonpaged_str { L"bar" }; 
```
- `ktl::list` supports allocations either from the ordinary pool allocations, or lookaside lists. Some helper templates are defined, to simplify specifying the correct block size for the lookaside allocations:
```C++
ktl::list<int> list1; // Create a new list, using ktl::paged_pool_allocator
ktl::paged_lookaisde_list<int> list2; // Create a new list, using ktl::paged_lookaside_allocator
ktl::list<int, ktl::nonpaged_pool_allocator> list3; // Create a new list, using ktl::nonpaged_pool_allocator
ktl::nonpaged_lookaside_list<int> list4; // Create a new list, using ktl::nonpaged_lookaside_allocator
```