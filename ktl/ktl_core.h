#pragma once

// NTDDK must be included before WDF, else you get odd build failures.
extern "C"
{
#pragma warning(push)
#pragma warning(disable: 4471)
#include <ntddk.h>

#define NOMINMAX
#include <wdf.h>
#pragma warning(pop)
}

#include "ktl_crt.h"
#include "ktl_config.h"

#define KTL_POOL_TAG 'LTSK'

#define KTL_LOG_MSG(level, fmt, ...) DbgPrintEx(DPFLTR_DEFAULT_ID, level, "[KTL] " __FUNCTION__ "(%d): " fmt, __LINE__, __VA_ARGS__)

#define KTL_LOG_ERROR(fmt, ...) KTL_LOG_MSG(DPFLTR_ERROR_LEVEL, fmt, __VA_ARGS__)
#define KTL_LOG_TRACE(fmt, ...) KTL_LOG_MSG(DPFLTR_TRACE_LEVEL, fmt, __VA_ARGS__)

#define KTL_REQUIRE_SUCCESS(call) do { NTSTATUS _callRet = ##call; if (NT_ERROR(_callRet)) { KTL_LOG_ERROR("call failed: %d\n", _callRet); } } while(0)
#define KTL_REQUIRE_NOTNULL(p) do { } while (0)
#define KTL_LOG_WARNING(x)

#if KTL_TRACE_COPY_CONSTRUCTORS
#define KTL_TRACE_COPY_CONSTRUCTOR	KTL_LOG_TRACE("copy constructing\n");
#else
#define KTL_TRACE_COPY_CONSTRUCTOR
#endif

#if KTL_TRACE_COPY_ASSIGNMENTS
#define KTL_TRACE_COPY_ASSIGNMENT	KTL_LOG_TRACE("copy assigning\n");
#else
#define KTL_TRACE_COPY_ASSIGNMENT
#endif

