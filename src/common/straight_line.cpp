#include "straight_line.h"
#include <stdarg.h>
#include "renderer.h"

StraightLine::StraightLine(Renderer* renderer, unsigned int count, ...) {
	va_list pointList;
	va_start(pointList, count);

	while(count > 0)
	{
		points.push_back(va_arg(pointList, Point));
		count--;
	}

	va_end(pointList);
};

bool StraightLine::isBezierCurve()
{
	return false;
}
