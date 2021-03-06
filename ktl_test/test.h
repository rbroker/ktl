#pragma once

#include "common.h"

#define ASSERT_TRUE(x, msg) do { if (!(##x)) { LOG_ERROR("Assertion Failed: %s (%s)\n", #x, msg); return false; } } while(0)
