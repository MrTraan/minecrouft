#include "logs.h"
#include "sys.h"
#include <imgui/imgui.h>

namespace ng {

Console & GetConsole() {
	static Console instance;
	return instance;
}

void Console::Draw() {
	ImGui::Begin( "Console" );
	ImGui::BeginGroup();
	ImGui::BeginChild( ImGui::GetID( "CONSOLE ID" ) );

	for ( auto const & log : logs ) {
		if ( log.severity == LogSeverity::LOG_ERROR ) {
			ImGui::TextColored( ImVec4( 255.0f, 0.0f, 0.0f, 255.0f ), "%s", log.text.c_str() );
		} else {
			ImGui::Text( "%s", log.text.c_str() );
		}
	}

	ImGui::EndChild();
	ImGui::EndGroup();
	ImGui::End();
}

void Console::PrintLog( const char * text, LogSeverity severity ) {
	Log newline{ text, severity, SysGetTimeInMicro() };
	logs.push_back( newline );
}

}; // namespace ng
