#include "sys.h"

#if defined( _WIN32 )
#include <Windows.h>
#endif


namespace ng {

static int64 clockTicksPerSecond = 0;
static int64 clockTicksAtStartup = 0;

void InitSys() {
#if defined( _WIN32 )
	LARGE_INTEGER li;
	QueryPerformanceFrequency( &li );
	clockTicksPerSecond = li.QuadPart;
	QueryPerformanceCounter( &li );
	clockTicksAtStartup = li.QuadPart;
#elif defined( __linux )
	NG_UNSUPPORTED_PLATFORM
#elif defined( __APPLE__ )
	NG_UNSUPPORTED_PLATFORM
#else
	NG_UNSUPPORTED_PLATFORM
#endif
}

int64 SysGetTimeInMicro() {
#if defined( _WIN32 )
	LARGE_INTEGER li;
	QueryPerformanceCounter( &li );
	int64 ticks = li.QuadPart - clockTicksAtStartup;
	return ticks * 1000000 * clockTicksPerSecond;
#elif defined( __linux )
	NG_UNSUPPORTED_PLATFORM
#elif defined( __APPLE__ )
	NG_UNSUPPORTED_PLATFORM
#else
	NG_UNSUPPORTED_PLATFORM
#endif
}

float SysGetTimeInMs() {
	return (float)SysGetTimeInMicro() / 1000.0f;
}

}; // namespace ng
