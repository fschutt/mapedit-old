#include "renderer.h"

#include <proj_api.h>
#include <vector>
#include <utility>
#include <map>
#include <algorithm>

#include "print_log_console.h"
#include "errors.h"
#include "messages.h"
#include "static_text.h"
#include "glm/ext.hpp"
#include "map_defs.h"
#include "polygon.h"
#include "jobmanager.h"

//Renderer is a class designed for making a framebuffer, which can be later on
//swapped to the screen.
Renderer::Renderer(Style* styleToRender)
{
	//glXMakeContextCurrent(display, None, None, context);

	/*
		RGB to CMYK conversion formula

		The R,G,B values are divided by 255 to change the range from 0..255 to 0..1:
		R' = R/255
		G' = G/255
		B' = B/255

		The black key (K) color is calculated from the red (R'), green (G') and blue (B') colors:

		K = 1-max(R', G', B')

		The cyan color (C) is calculated from the red (R') and black (K) colors:

		C = (1-R'-K) / (1-K)

		The magenta color (M) is calculated from the green (G') and black (K) colors:

		M = (1-G'-K) / (1-K)

		The yellow color (Y) is calculated from the blue (B') and black (K) colors:

		Y = (1-B'-K) / (1-K)
	*/

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if (GLEW_OK != err) {
		//Problem: glewInit failed, something is seriously wrong
		printErr(ERR_X_NO_GLEW);
		printErr((char*)glewGetErrorString(err));
		return;
	}

	// Get & checkOpenGL version
	int OpenGLVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);
	if(OpenGLVersion[0] < 3)
	{
		printErr(ERR_OPENGL_VERSION_INVALID);
		return;
	}

	//Setup renderer for orthographic rendering
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(0.0f, width, height, 0.0f, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);

	//COMPILE ALL SHADERS HERE
	// Shaders (must be formatted this way to make preprocessor happy)
    const GLchar* vertexShaderSource =
    #include "basic.vert"
    ;
    const GLchar* fragmentShaderSource =
    #include "basic.frag"
    ;

    OpenGLShader quadShader(vertexShaderSource, fragmentShaderSource);
    if(!(quadShader.success)){
        printErr((const char*)quadShader.infoLog);
        return;
    }

    shaders.insert(std::pair<const char*, OpenGLShader>("quadShader", quadShader));

    /*---------------------- COMPILE TEXT SHADER ----------------------*/

	vertexShaderSource =
    #include "text_shader.vert"
    ;
    fragmentShaderSource =
    #include "text_shader.frag"
    ;

	OpenGLShader textShader(vertexShaderSource, fragmentShaderSource);
    if(!(textShader.success)){
        printErr((const char*)textShader.infoLog);
        return;
    }

    //OpenGL 3.0 bind an attribute vec4(vertexX, vertexY, u,, v) to named location so the vertex shader can refer to it
    glBindAttribLocation(textShader.shaderProgram, 0, "TexturePlusUV");

	shaders.insert(std::pair<const char*, OpenGLShader>("textShader", textShader));
}

OpenGLShader* Renderer::getShaderByName(const char* shaderName)
{

    if(!shaders.empty()){
		std::map<const char*,OpenGLShader>::iterator i = shaders.find(shaderName);
		if(i != shaders.end()){
			return &i->second;
		}else{
			printf("Shader %s not found.", shaderName);
		}
    }else{
		printErr("Shaders are not compiled yet.");
    }

	return nullptr;
}

std::vector<Point> Renderer::recalculateBezierCurve(BezierCurve& b)
{
	std::vector<Point> fail;
    if(zoomLevelHasChanged){
        //TODO: Layer invisible and early AABB testing
        if(b.handles.empty()){
			printErr("No handles for recalculation.");
			return fail;
        }
		return b.solveForPoints();
    }
}

bool Renderer::isPointInBezierCurve(Point& p)
{
    if(p.line != nullptr && p.line->isBezierCurve()){
		return true;
    }

	return false;
}

std::vector<Point*> Renderer::getTwoNextHandles(Point& p)
{
	Line* l = p.line;
	std::vector<Point*> fail;

    if(l == nullptr){
		//point has no line attached
        return fail;
    }

    if(l->isBezierCurve()){
        BezierCurve& curve = static_cast<BezierCurve&>(*l);
		if(!curve.solvedPoints.empty()){
			for(unsigned int i = 0; i < curve.solvedPoints.size(); i++){
				//The point was clickable so it must lie somewhere on the curve
                //TODO !!!

			}
		}else{
			return fail;
		}
    }else{
        StraightLine& line = static_cast<StraightLine&>(*l); //downcast
        for(unsigned int i = 0; i < (line.points.size()) -1; i++){
			//Get direction of line
			double deltaX = line.points.at(i+1).x - line.points.at(i).x;
			double deltaY = line.points.at(i+1).y - line.points.at(i).y;
			double runRise = deltaY / deltaX;
			//Check if point is even contained in rectange between points
			//TODO !!!
        }
    }
}

void Renderer::recalculateCurrentArea(Job::Area& area){
	double areaWidth = area.EastBoundingCoordinate - area.WestBoundingCoordinate;
	double areaHeight = area.NorthBoundingCoordinate - area.SouthBoundingCoordinate;

	if(areaWidth < 0){ areaWidth = area.WestBoundingCoordinate - area.EastBoundingCoordinate; }
	if(areaHeight < 0){	areaHeight = area.SouthBoundingCoordinate - area.NorthBoundingCoordinate; }

	area.deltaNorthSouth = areaHeight;
	area.deltaWestEast = areaWidth;

	renderHeight = area.paperHeightMM * renderDPI;
	renderWidth = area.paperWidthMM * renderDPI;
	return;
}

void Renderer::areaToScreenPixels(Job::Area& area, unsigned int dpi)
{
	renderHeight = area.deltaNorthSouth / area.scale * dpi;
    renderWidth = area.deltaWestEast / area.scale * dpi;
}

Point Renderer::getClosestPointAtScreenLocation(Job::Area& area, int x, int y, int screenWidth, int screenHeight)
{
	//Get the area the user is currently looking at
    double xPercentage = x / screenWidth;
    double yPercentage = y / screenHeight;

    double clickWorldCoordX = area.WestBoundingCoordinate + (area.deltaWestEast * xPercentage);
    double clickWorldCoordY = area.SouthBoundingCoordinate + (area.deltaNorthSouth * yPercentage);

    return Point(clickWorldCoordX, clickWorldCoordY); // world position
}

std::vector<Point> Renderer::sortPoints()
{
   std::vector<Point> tempPoints = points;
   std::sort(tempPoints.begin(), tempPoints.end());
   return tempPoints;
}

void* Renderer::draw(void* context)
{
	JobManager::ThreadParameters* args = (JobManager::ThreadParameters*) context;
	((Renderer*)args->renderer)->recalculateCurrentArea(args->area);
	((Renderer*)args->renderer)->areaToScreenPixels(args->area);
	((Renderer*)args->renderer)->draw(args->area);
    pthread_exit(NULL);
}

void* Renderer::draw(Job::Area& areaToRender)
{
	printf("Drawing now!\n");
	printf("AREA:\tEAST: %f\tWEST: %f\tNORTH: %f\tSOUTH: %f\n!", areaToRender.EastBoundingCoordinate,
			areaToRender.WestBoundingCoordinate, areaToRender.NorthBoundingCoordinate, areaToRender.SouthBoundingCoordinate);

	printf("The resulting image will be %d pixels wide and %d pixels high.\n", renderWidth, renderHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//TODO: Optimize this
	if(zoomLevelHasChanged || renderParamsChanged){
		/*
		for(BezierCurve b : curves){
			recalculateBezierCurve(b);
		}
		*/
		//Regenerate projection matrix (for font)
		projection = glm::ortho(0.0f, static_cast<GLfloat>(areaToRender.paperWidthMM), 0.0f, static_cast<GLfloat>(areaToRender.paperHeightMM), -1.0f, 1.0f);
	}

	//Transform points
	//glm::mat3 modelMatrix(areaToRender.deltaNorthSouth, areaToRender.deltaWestEast, layer.zIndex);
	//glm::mat3 projectionMatrix;
	//glm::mat3 viewMatrix;


	//A bit complicated, see map_defs.h
	//multiPolygons = vector of MultiPolygon
	//MultiPolygon = map of RELATION and Polygon*
	//Polygon has lines = vector of Line*
	//Line has points = vector of Points*
	//Point has x and y

	//BezierHandles are converted into Points
/*
	printInfo("Points: ");
	for(Point p : points){
		printf("%f\t%f\n", p.x, p.y);
	}

	printInfo("Lines: ");
	for(unsigned int i = 0; i < lines.size(); i++){
		printf("\tLine %d referencing points:", i);
		for(unsigned int j = 0; j < lines.at(i).points.size(); j++){
			printf("\t%f\t%f\n", lines.at(i).points.at(j)->x, lines.at(i).points.at(j)->y);
		}
	}

	printInfo("Polygons: ");
	for(unsigned int i = 0; i < polygons.size(); i++){
		printf("\tPolygon %d referencing lines:", i);
		for(unsigned int j = 0; j < polygons.at(i).lines.size(); j++){
			printf("\t\tLine %d referencing points:", j);
			for(unsigned int k = 0; k < polygons.at(i).lines.at(j)->points.size(); k++){
				printf("\t\t%f\t%f\n", polygons.at(i).lines.at(j)->points.at(k)->x, polygons.at(i).lines.at(j)->points.at(k)->y);
			}
		}
	}

	printInfo("MultiPolygons: ");
	for(unsigned int i = 0; i < multiPolygons.size(); i++){
		printf("\tMultiPolygon %d referencing polygons:", i);
		for(unsigned int j = 0; j < multiPolygons.at(i).polygons.size(); j++){
			printf("\t\tPolyon %d referencing lines:", j);
			for(unsigned int k; k < multiPolygons.at(i).polygons.at(j).first->lines.size(); k++){
				printf("\t\Line %d referencing points:", k);
				for(unsigned int l = 0; l < multiPolygons.at(i).polygons.at(j).first->lines.at(k)->points.size(); l++){
					printf("\t\t%f\t%f\n", multiPolygons.at(i).polygons.at(j).first->lines.at(k)->points.at(l)->x, multiPolygons.at(i).polygons.at(j).first->lines.at(k)->points.at(l)->y);
				}
			}
		}
	}


*/
	//OpenGL rendering, main rendering loop
	//Draw points, lines, etc.
	glViewport(0, 0, renderWidth, renderHeight);
	/*
	OpenGLShader* textShader = getShaderByName("textShader");
	if((textShader != nullptr) && textShader->success){
		glUseProgram(textShader->shaderProgram);
		for(StaticText t : texts){
			t.draw(t.text, 0.0f, 0.0f, 1.0f, t.color, projection, textShader->shaderProgram);
		}
	}

	OpenGLShader* quadShader = getShaderByName("quadShader");
	if((quadShader != nullptr) && quadShader->success){
		glUseProgram(quadShader->shaderProgram);
	}

	//Clear all
	//Do this after drawing
	texts.clear();
	points.clear();
	straightLines.clear();
	polygons.clear();
	multiPolygons.clear();
	*/
	pthread_exit(NULL);
}

void* Renderer::draw(Job::Area& areaToRender, OUTPUT_MODE mode, const char* filePath)
{
	pthread_exit(NULL);
	//TODO: Output to file
}

Renderer::~Renderer()
{
}
