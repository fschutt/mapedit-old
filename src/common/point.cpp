#include "point.h"
#include "bezierhandle.h"

Point::Point(double x, double y) : x(x), y(y)
{

}

Point::Point(const BezierHandle& other){
	x = other.x;
	y = other.y;
	line = other.line;
}

bool Point::operator==(const Point& other){
    return x == other.x && y == other.y;
}

bool Point::operator<(const Point& other) const {
	return x < other.x | y < other.y;
}

bool Point::operator>(const Point& other) const {
	return x > other.x | y > other.y;
}

bool Point::isBezierHandle()
{
	return false;
}
