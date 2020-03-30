#pragma once

#include "logs.h"
#include "sys.h"
#include "types.h"
#include <stdlib.h>

// TODO: Remove in retail
#define NG_ASSERT_ENABLED
#define NG_PROFILING

#ifdef NG_ASSERT_ENABLED
#define ng_assert( condition )                                                                                         \
	if ( !( condition ) ) {                                                                                            \
		ng::Errorf( "ASSERTION FAILED: " #condition "\n" );                                                            \
		DEBUG_BREAK;                                                                                                   \
	}
#else
#define ng_assert( condition )
#endif

#ifdef NG_PROFILING
#include "tracy/Tracy.hpp"
#define ng_alloc( size ) ng::profiledAlloc( size )
#define ng_free( ptr ) ng::profiledFree( ptr )
#else
#define ng_alloc( size ) malloc( size )
#define ng_free( ptr ) free* ptr );
#endif

namespace ng {
void Init();
void Shutdown();

#ifdef NG_PROFILING
void * profiledAlloc( size_t size );
void   profiledFree( void * ptr );
#endif

}; // namespace ng