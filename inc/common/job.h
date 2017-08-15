#pragma once

#include <gdal/gdal_priv.h>
#include <gdal/cpl_conv.h> // for CPLMalloc()
#include <vector>
#include <CL/cl.h>
#include <pthread.h>

#include "style.h"

struct Job{

    Job(){ };
    ~Job(){ };
	//Job identifier. Must be tracked in an external database.
	const char* GUID;
	//Which version of the program is needed to handle this job
	int programVersion;
    //IP of the issuing referrer
    int referrer;
    //Authentication code
    const char* authCode;
    //Mutex to lock for other threads
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	enum class JOB_STATUS{
		WAIT_FOR_START,
        WAIT_OVERPASS,
        WAIT_GENERALIZING,
        WAIT_HEIGHT_STITCHING,
        WAIT_TRANSFORM,
        WAIT_EXPORT,
        FINISHED
	} status = JOB_STATUS::WAIT_FOR_START;

	//Does the job need logging? For debugging purposes.
	bool jobNeedsLogging = true;

	//One job can have multiple areas that have the same style. This allows
	//batching stuff together later on. For now, one Job = one Area
	struct Area{
		pthread_mutex_t mutex;
		//PROJ4 projection
		const char* projection;
		//User-defined extent of the job on paper
		unsigned int paperWidthMM;
		unsigned int paperHeightMM;
		unsigned int scale;
		//Bounding box of job
		float WestBoundingCoordinate;
		float NorthBoundingCoordinate;
		float EastBoundingCoordinate;
		float SouthBoundingCoordinate;
		//Convenience: the user might move the area, we need the width and the height for
		//calculating aspect ratio and other stuff
		//NOTE(felix): North and south do not have to be north and south. Edge cases are polar regions and \
		//west / east problems along the (meridian -180deg) line
		double deltaWestEast;
		double deltaNorthSouth;
		//Is the scale fixed or may it be changed later on?
		bool fixedScale = false;
	};

	//Style to use for this job
	Style style;

	//Areas
	std::vector<Area> arealist;

    /*--------------- QUEUEING AND STATUS FUNCTIONS ---------------*/
    //Start the job. This must be called after setting up the job.
    //This function checks OpenCL availability and sets up the context
    void start();


    /*---------------- APPLICATION LOGIC FUNCTIONS ----------------*/

	//Execute a query to one of the overpass servers. Set server names in the config file.
    //Returns and OSM XML string (not implemented yet). Uses a new free port for a connection.
	void* getDataOverPassAPI(const char* query);

	//Parse a PBF format. This will copy the PBF file into memory, split it into 8K blocks
	//then pass these on asynchronously to the GPU for string parsing. Note that no database system like
	//PostgreSQL or SpatiaLite is required.
	void* parsePBF(const char* filePath);

	//Applies a low-pass filter to a dataset of vector / polygon data.
	//Works by processing each polygon seperately and tracing the angle from the start (lines)
	//or a random point (polygon).
	void* applyLowPassFilter(GDALDataset* vectorDataset);

	//Adds a GPX Track if the style has a <layer attribute="gpxLayer></layer>
	//Returns false if the style is not configured for having a GPX / Overlay layer
	bool addGPXTrack(GDALDataset* gpxTrack);

	//Stitch various .hgt files together to one PNG file as well as gaussian blurring
	//and relief shading to make a nice height relief image
	//Requires OpenCL for fast image calculation
	void* stitchHeightTerrains();

private:
	//Helper function to determin the final area by transforming the given points, a projection
	//and if the area or the scale should be fixed (depending of projection there can be small discrepancies)
	void determineArea(Area& originialArea);

	//OpenCL helper functions
	bool  helperCLCheckError(cl_int& errorNr);
	std::string helperCLGetPlatformName(cl_platform_id& id);
	std::string helperCLGetDeviceName(cl_device_id& id);
}
;
