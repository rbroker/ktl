#include <mutex>

#include <stdarg.h>

#include "common.h"

/// <summary>
/// DbgPrintEx triggers a RECURSIVE_NMI under VirtualBox when print statements are executed in parallel,
/// so we guard prints with a global lock in case the debugger is connected.
/// </summary>
void SerializingDebugPrint(ULONG componentId, ULONG level, PCSTR format, ...)
{
	static ktl::mutex m{};

	va_list args;
	va_start(args, format);

	ktl::scoped_lock lock(m);

	DbgPrintEx(componentId, level, format, args);

	va_end(args);
}