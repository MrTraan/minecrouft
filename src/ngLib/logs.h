#pragma once

#include "types.h"

#include <string>
#include <vector>
#include <mutex>

using ngString = std::string;

namespace ng {
enum LogSeverity {
	LOG_INFO,
	LOG_ERROR,
	NUM_LOG_SEVERITY, // Keep me at the end plz
};

void Printf( const char * fmt, ... );
void Errorf( const char * fmt, ... );
void LogV( const char * fmt, va_list args, LogSeverity severity );

struct Console {
	struct Log {
		ngString    text;
		LogSeverity severity;
		int64 timestamp;
	};

	std::vector< Log > logs;
	std::mutex mutex;

	void PrintLog( const char * text, LogSeverity severity );
	void Draw();
};

Console & GetConsole();

}; // namespace ng