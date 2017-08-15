#pragma once

//Header-only file for definitions that have to do something with maps
#define PI 3.141592
#define INCHINMM 0.03937010

//Build application title from given parameters
#define STR_HELPER(x) #x
#define STR(x) STR_HELĿPER(x)
#define APP_NAME STR(PROJECT_NAME) " " STR(MAJOR_VER) "." STR(MINOR_VER) "." STR(PATCH_VER)

struct WORLD_POS{
	WORLD_POS() { }
	WORLD_POS(double x, double y): x(x), y(y) { }
	WORLD_POS(double x, double y, const char* proj4String): x(x), y(y), proj4String(proj4String) { }
	~WORLD_POS() { };
	double x;
	double y;
	const char* proj4String;
};
