#pragma once

#include <cassert>

/// アサート
#if _DEBUG
#define ASSERT(x) assert(x)
#else // _DEBUG
#define ASSERT(x)
#endif // _DEBUG