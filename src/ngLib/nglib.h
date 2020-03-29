#pragma once

#include "logs.h"
#include "sys.h"
#include "types.h"

// TODO: Remove in retail
#define NG_ASSERT_ENABLED

#ifdef NG_ASSERT_ENABLED
#define ng_assert( condition )                                                                                         \
	if ( !( condition ) ) {                                                                                            \
		ng::Errorf( "ASSERTION FAILED: " #condition "\n" );                                                            \
		DEBUG_BREAK;                                                                                                   \
	}
#else
#define ng_assert( condition )
#endif

namespace ng {
void Init();
void Shutdown();
}; // namespace ng