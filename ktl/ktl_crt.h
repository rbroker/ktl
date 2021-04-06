#pragma once

#ifndef KTL_OMIT_CRT_STUB
namespace ktl
{
	/// <summary>
	/// We don't have a typical CRT in Kernel Mode. As such, we need to take manual control over
	/// dynamic initialization. This method must be called at the very start of the DriverEntry
	/// routine to ensure all dynamic globals are correctly constructed.
	/// This method is not thread safe.
	/// Even if you call this method, you'll need to ignore linker warning LNK4210, else to avoid:
	/// warning LNK4210: .CRT section exists; there may be unhandled static initializers or terminators
	/// </summary>
	/// <returns>true on successful initialization, otherwise false.</returns>
	[[nodiscard]] bool initialize_runtime();

	/// <summary>
	/// We don't have a typical CRT in Kernel Mode. This method must be called at the end_ end_ of
	/// the DriverUnload routine, to ensure all dynamic globals are correctly destroyed.
	/// This method is not thread safe.
	/// Even if you call this method, you'll need to ignore linker warning LNK4210, else to avoid:
	/// warning LNK4210: .CRT section exists; there may be unhandled static initializers or terminators
	/// </summary>
	void unload_runtime();
}

int __cdecl atexit(void(__cdecl* func)(void));
#endif