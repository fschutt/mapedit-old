#pragma once

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <utility>

#include "fontface.h"
#include "opengl_shader.h"
#include "layer.h"

//NOTE(felix): For clarity, in these files, a Renderer* exist
#include "point.h"
#include "straight_line.h"
#include "beziercurve.h"
#include "bezierhandle.h"
#include "multi_polygon.h"
#include "static_text.h"
#include "job.h"
#include "jobmanager.h"

struct Renderer
{
        Renderer(Style* styleToRender);

        //Since shaders are not likely going to change, we compile them in the Renderer initializer
        ~Renderer();

        //TODO(felix): DPI scaling, move vector stuff out of here
        glm::mat4 projection;

        enum class OUTPUT_MODE {
            OUTPUT_TIFF,
            OUTPUT_PNG,
            OUTPUT_EPS,
            OUTPUT_PDF
        };

        void addText(const char* text, unsigned int size, FontFace* face, glm::vec3& color, unsigned int& rotation);

        std::vector<Point> recalculateBezierCurve(BezierCurve& b);
        //Is the point in a straight line or in a bezier curve
        bool isPointInBezierCurve(Point& p);
        //Get the midpoint between two handles
        Point& getMidpointTwoBezierHandles(BezierHandle& first, BezierHandle& next);
        //Get the next two handles
        std::vector<Point*> getTwoNextHandles(Point& p);
        //Get the adress of the point that is the closest to the cursor
        Point getClosestPointAtScreenLocation(Job::Area& area, int x, int y, int screenWidth, int screenHeight);
        //Sort vector horizontal then vertical
        std::vector<Point> sortPoints();

        //Helper function for pthread, calls other draw function
        static void* draw(void* context);
        //Draw on screen
        void* draw(Job::Area& areaToRender);
        //Draw to file
        void* draw(Job::Area& areaToRender, OUTPUT_MODE mode, const char* filePath);

        Style* styleToRender;

        bool renderParamsChanged = false;
        bool zoomLevelHasChanged = false;
        const char* proj4EPSGString; //Projection of the map
        unsigned int renderDPI = 300;
private:
        //Recalculate the area the user (or renderer) is currently looking at.
        //This function should fix problems with flipped areas due to projection problems
        void recalculateCurrentArea(Job::Area& area);
        //Sets the render width and height from given DPI
        void areaToScreenPixels(Job::Area& area, unsigned int dpi=96);
        //The width and height in pixel to render
        double renderWidth, renderHeight;

        OpenGLShader* getShaderByName(const char* shaderName);

        GLuint VAO;
        GLuint pointsVBO;
        GLuint linesVBO;
        GLuint polygonsVBO;
        GLuint multipolygonsVBO;
        GLuint textsVBO;


        std::vector<BezierCurve> curves;
        std::vector<Point> points;
        std::vector<StaticText> texts;
        std::vector<StraightLine> straightLines;
        std::vector<MultiPolygon> multiPolygons;
        std::vector<Polygon> polygons;
        //NOTE(felix): Other classes should not be allowed to access these vectors directly
        //because that would quickly create problems. Inside the functions we "lose" the pointer to the allocated
        //memory (we store it inside these following vectors). On deconstruction time, we go over all Points, see if they
        //have any pointers connected. Because Points are the only objects that do not depend on something else.
        //At rendering time it's the other way around. Here we go over the bezierCurvers, handles, polygons etc.
        //and see what points are attached to them. It is important not to lose any pointers in this chain
        /*
        std::map<BezierCurve, std::vector<BezierHandle>>        bezierHandles;          //List with all bezier handles
        std::vector<Point>                                      points;                 //Huge list where all the points go
        std::vector<Polygon>                                    polygons;
        std::vector<MultiPolygon>                               multiPolygons;
        std::vector<StaticText>                                 texts;                  //Huge list with all texts
        std::vector<BezierCurve>                                curves;
        StraightLine& addBezier(BezierCurve& bezierCurve);
        */
        std::map<const char*, OpenGLShader> shaders;
};
