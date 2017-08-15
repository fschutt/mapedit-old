#pragma once
#include "renderer.h"

//Polygon
struct Polygon{
    Polygon();
    ~Polygon();
	//The line has to know if it is belonging to a polygon
    MultiPolygon* multiPolygon = nullptr;
    std::vector<Line*> lines;
    Renderer* renderer;
};
