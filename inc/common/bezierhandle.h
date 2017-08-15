#pragma once
#include "point.h"

struct BezierHandle : public Point{
	virtual bool isBezierHandle();			//Is the point a bezierHandle: true
	BezierCurve* curve = nullptr;			//Which curve does the bezier handle belong to?
};
