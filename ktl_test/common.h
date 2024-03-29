#pragma once

extern "C"
{
#pragma warning(push)
#pragma warning(disable: 4471)
#define NOMINMAX
#include <ntddk.h>
#include <wdf.h>
#pragma warning(pop)
}

#include <ktl_core.h>

#define LOG_MSG(level, fmt, ...) DbgPrintEx(DPFLTR_DEFAULT_ID, level, "[KTLTEST] " __FUNCTION__ "(%d): " fmt, __LINE__, __VA_ARGS__)

#define LOG_ERROR(fmt, ...) LOG_MSG(DPFLTR_ERROR_LEVEL, fmt, __VA_ARGS__)
#define LOG_WARNING(fmt, ...) LOG_MSG(DPFLTR_WARNING_LEVEL, fmt, __VA_ARGS__)
#define LOG_TRACE(fmt, ...) LOG_MSG(DPFLTR_TRACE_LEVEL, fmt, __VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG_MSG(DPFLTR_INFO_LEVEL, fmt, __VA_ARGS__)