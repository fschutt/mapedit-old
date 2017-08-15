#pragma once
#include <vector>
#include "line.h"

struct Renderer;
struct Point;
struct Polygon;
struct BezierCurve;

//Straight line
struct StraightLine : public Line {
    StraightLine() { };
    StraightLine(Renderer* renderer, unsigned int count, ...);
    ~StraightLine(){ };

    virtual bool isBezierCurve();
    std::vector<Point> points;
    Renderer* renderer;
	//The line has to know if it is belonging to a polygon
    Polygon* polygon = nullptr;
};
