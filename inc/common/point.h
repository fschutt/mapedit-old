#pragma once
#include "line.h"

struct StraightLine;
struct BezierCurve;
struct BezierHandle;

struct Point{

	Line* line = nullptr;				//Is the point connected to a line?
	double x;
    double y;

    Point(){ };
	Point(double x, double y);
	Point(const BezierHandle&);			//construct point from BezierHandle (loses information)
	bool operator<(const Point& other) const;	//for sorting a vector of point objects by x then by y
	bool operator>(const Point& other) const;	//for sorting a vector of point objects by x then by y
    bool operator==(const Point& other);
    virtual bool isBezierHandle();		//Is the point a bezier handle?
    virtual ~Point(){ };
};
