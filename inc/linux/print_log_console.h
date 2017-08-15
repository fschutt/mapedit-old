#pragma once

//Linux implementation of colored console output & logging
#include <stdio.h>

#define LINUX_CONSOLE_TEXT_RESET   		    "\x1b[0m"
#define LINUX_CONSOLE_TEXT_LIGHT_RED		"\x1b[91m"
#define LINUX_CONSOLE_TEXT_LIGHT_GREEN		"\x1b[92m"
#define LINUX_CONSOLE_TEXT_LIGHT_BLUE		"\x1b[94m"

//Print to console using colors
inline void printMsg(const char* message){
	printf(LINUX_CONSOLE_TEXT_RESET "%s" LINUX_CONSOLE_TEXT_RESET "\n", message);
}

inline void printInfo(const char* message){
	printf(LINUX_CONSOLE_TEXT_LIGHT_BLUE "%s" LINUX_CONSOLE_TEXT_RESET "\n", message);
}

inline void printErr(const char* message){
	printf(LINUX_CONSOLE_TEXT_LIGHT_RED "%s" LINUX_CONSOLE_TEXT_RESET "\n", message);
}

inline void printSuccess(const char* message){
	printf(LINUX_CONSOLE_TEXT_LIGHT_GREEN "%s" LINUX_CONSOLE_TEXT_RESET "\n", message);
}
