#pragma once

#include <cassert>

/// �A�T�[�g
#if _DEBUG
#define ASSERT(x) assert(x)
#else // _DEBUG
#define ASSERT(x)
#endif // _DEBUG