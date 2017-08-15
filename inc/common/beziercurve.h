#pragma once

#include "glm/glm.hpp"
#include <stdarg.h>
#include <vector>
#include "bezierhandle.h"
#include "line.h"

struct Renderer;

struct BezierCurve : public Line
{
        BezierCurve();
        BezierCurve(Renderer* renderer);
        BezierCurve(Renderer* renderer, unsigned int count, BezierHandle first, ...);
        ~BezierCurve();

        virtual bool isBezierCurve();

        //Array of handles for faster access during solveForPoints
        std::vector<BezierHandle> handles;

        //Final vector to return
        std::vector<Point> solvedPoints;

        //The basic objects are points (BezierHandle just holds a reference to a point)
        //The renderer manages all of these points.
        Renderer* renderer;

        double distanceTolerance = 0.03f*0.03f;
        double angleTolerance = 5.0f; //degrees

        std::vector<Point> solveForPoints();
private:
        void recursiveSolveForPoints(Point& p1, Point& p2, Point& p3);
        void recursiveSolveForPoints(Point& p1, Point& p2, Point& p3, Point& p4);

};
