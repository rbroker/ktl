#include <mutex>

#include <stdarg.h>

#include "common.h"

/// <summary>
/// DbgPrintEx triggers a RECURSIVE_NMI under VirtualBox when print statements are executed in parallel,
/// so we guard prints with a global lock in case the debugger is connected.
/// </summary>
void SerializingDebugPrint(ULONG componentId, ULONG level, PCSTR format, ...)
{
#ifdef _DEBUG
	static ktl::mutex m{};
#endif

	va_list args;
	va_start(args, format);

#ifdef _DEBUG
	ktl::scoped_lock lock(m);
#endif

	DbgPrintEx(componentId, level, format, args);

	va_end(args);
}