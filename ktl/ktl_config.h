#pragma once

/*
 * Causes pool allocations to be counted in/out. If allocations mismatch,
 * a warning will be printed when the ktl::unload_runtime is called.
 */
#if _DEBUG
#define KTL_TRACK_ALLOCATIONS 1
#endif

/*
 * Causes debug prints to be serialized behind a FAST_MUTEX, to prevent
 * recursive NMI issues on VirtualBox VMs.
 */
#define KTL_TARGET_VM 1