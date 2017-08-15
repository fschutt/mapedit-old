//Windows implementation of colored console output
#include <windows.h>

#define WIN_CONSOLE_TEXT_WHITE			"\x0F"
#define WIN_CONSOLE_TEXT_LIGHT_RED		"\x0C"
#define WIN_CONSOLE_TEXT_LIGHT_GREEN		"\x0A"

//Print to console using colors
//TODO: All of this. Delay until Win32 port.
inline void printMsg(const char* message){
	printf(message "\n");
}

inline void printInfo(const char* message){
	printf(message "\n");
}

inline void printErr(const char* message){
	printf(message "\n");
}

inline void printSuccess(const char* message){
	printf(message "\n");
}
