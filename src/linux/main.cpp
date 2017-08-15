#ifdef SERVER_BUILD
	#include "server.h"
#endif

#include <malloc.h>

#include "appwindow.h"
//DEBUG
#include "heightterrain.h"
#include "job.h"
#include "opengl_shader.h"
#include "beziercurve.h"
#include "fontface.h"
#include "print_log_console.h"
#include "jobmanager.h"
#include "map_defs.h"

void dumpMallinfo(void) {
  struct mallinfo m = mallinfo();
  printf("uordblks = %d\nfordblks = %d\n", m.uordblks, m.fordblks);
}

int main(int argc, char* argv[]) {

	//CLI Arguments
    if(argc > 1)
    {
        if(argc > 2){
			Server::getInstance().handleCMDArgs((unsigned int)argc, argv);
        }else{
			Server::getInstance().printHelp();
			return EXIT_SUCCESS;
        }
    }

    //HeightTerrain h;

    //does not work yet
    //h.determineFiles(0.0f, 1.0f, 2.0f, 3.0f, 300, 400);

	//Job j(true, nullptr);

	//OpenCL fails to compile kernel
	//j.applyLowPassFilter(nullptr);

	//PBF fails to load with valid size (do not use statbut.st_size) for determining file size
	//j.parsePBF("/home/felix/Downloads/great-britain-latest.osm.pbf");

    //AppWindow main_window(800, 600, APP_NAME, AppWindow::WindowStyle::WS_EX_APPWINDOW);
    //main_window.show();
    Server::getInstance().startOGL();

    int err;
    pthread_t jobThread1;

	if(JobManager::getInstance().addJob("mockdata/29_12_2016_21_57_03.xml") == JobManager::xmlParseError::JOB_NO_ERROR){
        err = pthread_create(&jobThread1, NULL, &JobManager::startNextJob, (void*)&JobManager::getInstance().jobs);
        if(err){ printf("Unable to create rendering thread1: %d\n", err);}
	}

    pthread_join(jobThread1, NULL);
    Server::getInstance().stopOGL();

    dumpMallinfo();




/*
	Point p1(0.1f, 0.38f);
    Point p2(0.59f, 0.22f);
    Point p3(0.33f, 0.45f);
    Point p4(0.75f, 0.39f);

    FontFace test;
    const char* renderText = "Liberation Sans Schrifttest gp";
    main_window.renderer->addText(renderText, &test, 24, glm::vec3(0.5f, 0.5f, 0.5f));

    BezierCurve temp1(5, &p1, &p1, &p1, &p1, &p1);
    BezierCurve temp2(4, &p1, &p3, &p2, &p4);
    BezierCurve temp3(3, &p1, &p1, &p1);
    BezierCurve temp4(2, &p1, &p2);
    BezierCurve temp5(1, &p1);
    BezierCurve temp6(6, &p1, &p3, &p2, &p4, &p2, &p3);
    BezierCurve temp7(7, &p1, &p3, &p2, &p4, &p2, &p3, &p4);
    BezierCurve temp8(8, &p1, &p3, &p2, &p4, &p2, &p3, &p4, &p1);

    printInfo("Solve for temp1!");
    temp1.solveForPoints();
    main_window.renderer->draw();

    printInfo("Solve for temp2!");
    temp2.solveForPoints();
    main_window.renderer->draw();

    printInfo("Solve for temp3!");
    temp3.solveForPoints();
    main_window.renderer->draw();

    printInfo("Solve for temp4!");
    temp4.solveForPoints();
    main_window.renderer->draw();

    printInfo("Solve for temp5!");
    temp5.solveForPoints();
    main_window.renderer->draw();

    printInfo("Solve for temp6!");
    temp6.solveForPoints();
    main_window.renderer->draw();

    printInfo("Solve for temp6!");
    temp7.solveForPoints();
    main_window.renderer->draw();

    printInfo("Solve for temp6!");
    temp8.solveForPoints();
    main_window.renderer->draw();
*/
	return EXIT_SUCCESS;
}
