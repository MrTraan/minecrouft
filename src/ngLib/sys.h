#pragma once

#include "types.h"

#define NG_UNSUPPORTED_PLATFORM static_assert( false, "Platform specific not handled here" );

#if defined( _WIN32 )
#define DEBUG_BREAK __debugbreak()
#elif defined( __linux )
#define DEBUG_BREAK __asm__ __volatile__( "int $0x03" )
#elif defined( __APPLE__ )
#include <signal.h>
#define DEBUG_BREAK raise( SIGTRAP )
#else
NG_UNSUPPORTED_PLATFORM
#endif

namespace ng {
void InitSys();

int64 SysGetTimeInMicro();
float SysGetTimeInMs();
};
