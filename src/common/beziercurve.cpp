#include "beziercurve.h"
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include "print_log_console.h"
#include "renderer.h"
#include "map_defs.h"

BezierCurve::BezierCurve(Renderer* renderer, unsigned int count, BezierHandle first, ...)
{
    //Bezier curve initialized with points
    renderer = renderer;

    //Put bezier handles in handles vector
    va_list optionalPoints;
    va_start(optionalPoints, first);
    while(count > 0){
        handles.push_back(va_arg(optionalPoints, BezierHandle));
        count--;
    }
    va_end(optionalPoints);

}

BezierCurve::BezierCurve(Renderer* renderer)
    : renderer(renderer)
{
    //Empty bezier curve
}

bool BezierCurve::isBezierCurve(){
    return true;
}

std::vector<Point> BezierCurve::solveForPoints()
{
    std::vector<Point> fail;
    //Solve bezier curve for points, put resulting Line and Point objects in renderer associated with this curve
    if(handles.empty())
    {
        printErr("Error: Bezier curve has no points.");
        return fail;
    }

    //Batch point in units of four to be evaluated
    unsigned int numHandles = handles.size();
    unsigned int numOddPoints = numHandles % 4;
    unsigned int numBezierCurves = (unsigned int) floorf(numHandles / 4);

    bool breakLoop = false;
    //Are there more than 4 points in the whole curve?
    bool lessThanFourPoints = (numBezierCurves == 0);

    if(lessThanFourPoints == false){
        //Temporary handles which are actually the midpoints between
        //http://www.algosome.com/articles/images/continuous-cubic-bezier-curve.jpg
        Point tempHandle(0.0f, 0.0f);
        //midpointHandle is just for passing
        Point midpointHandle(0.0f, 0.0f);

        //i=i+3 is intentional
        //the fourth point is replaced by tempHandle, which "glues" the bezier curve parts together
        for(int i=0; i < (int)numHandles && !breakLoop; i=i+3){
            if ((i-3) < 0){
                //First bezier curve part, doesn't matter how many points
                tempHandle.x = (handles[i+2].x - handles[i+1].x) / 2;
                tempHandle.y = (handles[i+2].y - handles[i+1].y) / 2;
                midpointHandle.x = (handles[i+1].x - handles[i].x) / 2;
                midpointHandle.y = (handles[i+1].y - handles[i].y) / 2;
                recursiveSolveForPoints(handles[i], midpointHandle, handles[i+1], tempHandle);
            }else{
                switch(numOddPoints){
                    case 0:{
                        //As an example
                        //4, 8, 12, 16 Points
                        //Special cases for first and last patch of 4 points
                        if(i+3 == (int)numHandles){
                            //Last bezier curve part
                            recursiveSolveForPoints(tempHandle, handles[i], handles[i+1], handles[i+2]);
                        }else{
                            //Some curve part in the middle
                            midpointHandle = tempHandle;
                            tempHandle.x = (handles[i+2].x - handles[i+1].x) / 2;
                            tempHandle.y = (handles[i+2].y - handles[i+1].y) / 2;
                            recursiveSolveForPoints(midpointHandle, handles[i], handles[i+1], tempHandle);
                        }
                    } break;
                    case 3:{
                        //7, 11, 15 points

                    } break;
                    case 2:{
                        //6, 10, 14 points
                        //recursiveSolveForPoints(&handles[i], &handles[i+1]);
                    } break;
                    case 1:{
                        //5, 9, 13 Points
                        //recursiveSolveForPoints(&handles[i], &handles[i+1]);
                    } break;
                }
            }
        }
    }else{
        //There are not more than 4 points in the curve
        //This means we don't have to do any "glueing"
        switch(numOddPoints){
        case 1:{
            //Point
            solvedPoints.push_back(Point(handles[0]));
        } break;
        case 2:{
            //Line
            solvedPoints.push_back(Point(handles[0]));
            solvedPoints.push_back(Point(handles[1]));
        } break;
        case 3:{
            //quadratic bezier curve
            recursiveSolveForPoints(handles[0], handles[1], handles[2]);
        } break;
        }
    }
}

//Quadratic bezier curve: split into two cubic bezier curves
void BezierCurve::recursiveSolveForPoints(Point& p1, Point& p2, Point& p3)
{
    Point p12(    (p1.x       +   (p2.x   - p1.x)/ 2),    (p1.y   + (p2.y - p1.y)/ 2)     );
    Point p23(    (p2.x       +   (p3.x   - p2.x)/ 2),    (p2.y   + (p3.y - p2.y)/ 2)     );

    recursiveSolveForPoints(p1,p12, p23, p3);
}

//Evaluating a cubic bezier curve MUST have four points!!!
void BezierCurve::recursiveSolveForPoints(Point& p1, Point& p2, Point& p3, Point& p4)
{
    //Optimized de Castellau / adaptive bezier function
    //see: http://www.antigrain.com/research/adaptive_bezier/index.html#toc0004

    //TODO: preliminary check if some points are the same
    bool startingPointsMatch = (p1 == p2);
    bool endingPointsMatch = (p3 == p4);
    bool middlePointsMatch = (p2 == p3);

    //If there is only one point
    if(startingPointsMatch && endingPointsMatch && middlePointsMatch){
        solvedPoints.push_back(p1);
        return;
    }

    //See this graphic for the point positions:
    //http://www.antigrain.com/research/adaptive_bezier/bezier06.gif
    //for example p1234 lies in the middle between p12 and p34
    Point p12(    (p1.x       +   (p2.x   - p1.x)/ 2),    (p1.y   + (p2.y - p1.y)/ 2)     );
    Point p23(    (p2.x       +   (p3.x   - p2.x)/ 2),    (p2.y   + (p3.y - p2.y)/ 2)     );
    Point p34(    (p3.x       +   (p4.x   - p3.x)/ 2),    (p3.y   + (p4.y - p3.y)/ 2)     );
    Point p123(   (p12.x      +   (p23.x  - p12.x)/ 2),   (p12.y  + (p23.y - p12.y)/ 2)   );
    Point p234(   (p23.x      +   (p34.x  - p23.x)/ 2),   (p23.y  + (p34.y - p23.y)/ 2)   );
    Point p1234(  (p123.x     +   (p234.x - p123.x)/ 2),  (p123.y + (p234.y - p123.y)/ 2) );

    //Estimate if the curve is flat
    double distX14 = p4.x - p1.x;
    double distY14 = p4.y - p1.y;

    double distVertical12 = fabs(((p2.x - p4.x)* distY14 - (p2.y - p4.y)* distX14));
    double distVertical34 = fabs(((p3.x - p4.x)* distY14 - (p3.y - p4.y)* distX14));

    if((distVertical12 + distVertical34)*(distVertical12 + distVertical34) < distanceTolerance * (distX14 * distX14 + distY14 * distY14))
    {
        solvedPoints.push_back(p1234);
        return;
    }

    //Calulate angle for points 2 and 3
    //NOTE(felix): atan = expensive operation
    double angle23 = atan2(p3.y - p2.y, p3.x - p2.x);
    double degAngle2 = fabs(angle23 - atan2(p2.y - p1.y, p2.x - p1.x));
    double degAngle3 = fabs(atan2(p4.y - p3.y, p4.x - p3.x) - angle23);

    //normalize angle
    if(degAngle2 >= PI){ degAngle2 = 2 * PI - degAngle2; }
    if(degAngle3 >= PI){ degAngle3 = 2 * PI - degAngle3; }

    //cusp condition
    if((degAngle2 + degAngle3 ) < angleTolerance){
        solvedPoints.push_back(p1234);
        return;
    }

    //Split bezier curve into two curves and solve from there
    recursiveSolveForPoints(p1, p12, p123, p1234);
    recursiveSolveForPoints(p1234, p234, p34, p4);
    return;
}

BezierCurve::~BezierCurve()
{
    //Iterate through all bezier points and delete memory
    //TODO: currently we have a huge memory leak!!!!
}
