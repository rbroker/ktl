# ktl - Kernel-mode Template Library
A tiny C++ Template Library for Windows Kernel-Mode. Not fit for production use, and only includes subset of STL-ish behaviour I find useful in personal projects.

## Stub CRT
In order to ensure proper handling of global C++ data, new, delete, etc. a partial CRT stub is included. Defining `KTL_OMIT_CRT_STUB` in the preprocessor will avoid this, but KTL still depends on a definition for all new/delete methods to be provided, and for `_fltused` to be defined if any floating point operations are in use (e.g. `set`).

`/ignore:4210` should be ignored in the linker operations, as we implement the static initializers & terminators: "warning LNK4210: .CRT section exists; there may be unhandled static initializers or terminators".

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
NTSTATUS
DriverUnload(
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
)
{
    ...

    // Unload the KTL runtime (run dynamic atexit, etc).
    ktl::unload_runtime();

    return STATUS_SUCCESS;
}
```

## Features
|Header|Feature|Note|
|--|--|---|
| algorithm | `find_if`, `equal_to`, `min`, `max` | |
| cstddef | `nullptr_t` | |
| cstdint | `int8_t` -> `uint64_t` | |
| kernel | `floating_point_state`, `auto_irp`, `safe_user_buffer` | `ktl::floating_point_state` is needed for using [x87 floating point](https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-kesaveextendedprocessorstate).
| limits | `<T>min`, `<T>max` | For everything in cstdint |
| list | `list<T>` | Based on kernel [LIST_ENTRY](https://docs.microsoft.com/en-us/windows/win32/api/ntdef/ns-ntdef-list_entry) |
| memory | `addressof`, `unique_ptr<T>`, `observer_ptr<T>`, `make_unique<T>` | |
| mutex | `scoped_lock`, `mutex` | No deadlock-avoiding lock, based on [FAST_MUTEX](https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/eprocess) |
| new | `new`, `delete`, `new[]`, `delete[]`, placement `new` | Normal STL new/deletes, plus new overloaded with `ktl::pool_type`. All news are non-throwing. |
| set | `set<T>` | *unordered* set implementation. |
| shared_mutex | `shared_lock`, `shared_mutex` | reader-writer locking based on [ERESOURCE](https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/introduction-to-eresource-routines) |
| string | `unicode_string` | No `string` or `wstring`, everything is UTF-16 [UNICODE_STRING](https://docs.microsoft.com/en-us/windows/win32/api/ntdef/ns-ntdef-_unicode_string). |
| string_view | `unicode_string_view` | For the performance-conscious [UNICODE_STRING](https://docs.microsoft.com/en-us/windows/win32/api/ntdef/ns-ntdef-_unicode_string) user. |
| type_traits | | Just enough for built-in features!
| utility | `pair<T1, T2>` | |
| vector | `vector<T>` | Fan favourite, probably far from optimised. |

## Differences from usermode STL
I've abused the STL header names, but this is not an STL reimplementation, and even the bits that look similar aren't intended to be remotely standards compliant. There's a number of change-points from a typical STL implementation to account for operating in kernel-mode, and without C++ exceptions. Some things possibly worth bearing in mind:

- The `new` header defines an overload of `operator new` which accepts a `ktl::pool_type` parameter. This allows allocations to come from either the paged, or unpaged pools.
    - Scalar and array versions of `make_unique` also support supporting the pool type.
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