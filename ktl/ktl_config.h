#pragma once

/*
 * Causes pool allocations to be counted in/out. If allocations mismatch,
 * a warning will be printed when the ktl::unload_runtime is called.
 */
#if _DEBUG
#define KTL_TRACK_ALLOCATIONS 1
#endif

/*
 * Print trace messages on copy-construction of objects.
 */
#if _DEBUG
#define KTL_TRACE_COPY_CONSTRUCTORS 0
#endif

/*
 * Print trace messages on copy-assignment of objects.
 */
#if _DEBUG
#define KTL_TRACE_COPY_ASSIGNMENTS 0
#endif