#include "job.h"
#include "print_log_console.h"

//IMPORTANT! The following macro is for supressing warnings about
//clCreateCommandQueue, which is depreceated, but not removed in OpenCL 2.0 by
//clCreateCommandQueueWithProperties. If this code does not need to run on
//OpenCL 1.1 devices anymore, remove the following macro.
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include "CL/cl.h"
#include "fileformat.pb.h"
#include "osmformat.pb.h"
#include "zlib.h"

#include <stdio.h>
#include <time.h>
#include <proj_api.h>

#include "renderer.h"
#include "map_defs.h"

void Job::start(){
	//TODO: Calculate final sizes of areas
}

void* Job::getDataOverPassAPI(const char* query)
{
	status = JOB_STATUS::WAIT_OVERPASS;
	return nullptr;
}

bool Job::addGPXTrack(GDALDataset* gpxTrack)
{
	//Load from memory - we need MEM driver
	unsigned int numbands = 0;
	unsigned int width = 300;
	unsigned int height = 500;

	char **papszOptions = NULL;
	GDALDriver *poDriver  = GetGDALDriverManager()->GetDriverByName("MEM");

	GDALDataset* poDstDS = poDriver->Create( "dummyname",(int)width, (int)height, numbands, GDT_Byte, papszOptions);
/*
	for(unsigned int b = 0; b < 3; b++)
	{
		char szPtrValue[128] = { '\0' };
		int nRet = CPLPrintPointer( szPtrValue,
		reinterpret_cast<void*>(abyRasters[b]), sizeof(szPtrValue) );
		szPtrValue[nRet] = 0;
		papszOptions = CSLSetNameValue(papszOptions, "DATAPOINTER", szPtrValue);
		poDstDS->AddBand(GDT_Byte, papszOptions);
	}
*/
	CSLDestroy(papszOptions);
/*
	//jpeg write test:
	{
		char **papszOptions2 = NULL;
		papszOptions2 = CSLSetNameValue( papszOptions2, "QUALITY", "40" );
		GDALDriver* jpegDriver = GetGDALDriverManager()->GetDriverByName("JPEG");
		GDALDataset* jpeg_ds = jpegDriver->CreateCopy("somename.jpg", poDstDS, false, papszOptions2, GDALDummyProgress, NULL);
		CSLDestroy(papszOptions2);
		GDALClose( (GDALDatasetH) jpeg_ds );
	}
*/
	GDALClose( (GDALDatasetH) poDstDS );
	return true;
}

void* Job::applyLowPassFilter(GDALDataset* vectorDataset)
{
   //Replace this test data with image data
	uint32_t dataItems = 7;
	size_t dataSize = (dataItems * sizeof(long));
    long testData[] = {1000, 1003, 8132, 9809, 93218, 45, 35};
	long otherData[] = {300, 500, 8008, 494, 123, 56, 89};
	//TODO: something is heavily wrong about this malloc (only 8 bytes, should be 56)
	long* finalData = (long*) malloc(sizeof(long) * 7);

	printf("Size of testData\t%d\n", sizeof(testData));
	printf("Size of otherData\t%d\n", sizeof(otherData));
	printf("Size of finalData\t%d\n", sizeof(*finalData));
	printf("dataSize\t%d\n", dataSize);
	printf("Size of long\t%d\n", sizeof(long));

	//OpenCL information for platform (OpenCL version) and device (physical device)
	//There may be more platforms available, but this doesn't matter for now
    cl_uint platformCount = 0;
	cl_uint deviceCount = 0;
	cl_platform_id platform;
	cl_device_id device;
	cl_command_queue commandQueue;
    cl_int error = CL_SUCCESS;
	cl_context context;
	cl_program program;
	cl_kernel kernel;

	cl_mem aBuffer;
	cl_mem bBuffer;
    static float two = 2.0f;

	const char* sources[1] = {
		#include "opencl/saxpy.cl"
		};

	const size_t lenghts[1] = { strlen(sources[0]) };

	//First platform, then device
	if(clGetPlatformIDs(1, &platform, &platformCount) == CL_SUCCESS){
		std::string platformString = helperCLGetPlatformName(platform);
		printf("OpenCL device:\t\t\t%s\n", platformString.c_str());
	}

    if(clGetDeviceIDs (platform, CL_DEVICE_TYPE_GPU, 1, &device, &deviceCount) == CL_SUCCESS){
        printSuccess("OpenCL devide type:\t\tGPU");
    }else if(clGetDeviceIDs (platform, CL_DEVICE_TYPE_ACCELERATOR, 1, &device, &deviceCount) == CL_SUCCESS){
        printSuccess("OpenCL devide type:\t\tACCELERATOR");
    }else if(clGetDeviceIDs (platform, CL_DEVICE_TYPE_ALL, 1, &device, &deviceCount) == CL_SUCCESS){
        printSuccess("OpenCL devide type:\t\tALL");
    }

    if(deviceCount != 0){
		std::string deviceString = helperCLGetDeviceName(device);
		printf("OpenCL platform:\t%s\n", deviceString.c_str());
	}else{
		printErr("No OpenCL device found.");
		return nullptr;
    }

    //Create OpenCL context
    const cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platform), 0, 0 };
    context = clCreateContext(contextProperties, deviceCount, &device, nullptr, nullptr, &error);
	if(!helperCLCheckError(error)){ printErr("\nFailed to create OpenCL context!"); clReleaseContext(context); return nullptr;}

	// Create a command queue
    commandQueue = clCreateCommandQueueWithProperties(context, device, NULL, &error);
    if(!helperCLCheckError(error)){ printf("Could not create an OpenCL Command Queue\n"); return nullptr; }

	//Allocate memory on the GPU
	aBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(testData), &testData, &error);
	if(!helperCLCheckError(error)){ printErr("Failed to create Buffer A!"); return nullptr;}
	bBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(otherData), &otherData, &error);
	if(!helperCLCheckError(error)){ printErr("Failed to create Buffer B!"); return nullptr;}

	//Create program
	program = clCreateProgramWithSource(context, 1, sources, lenghts, &error);
	if(!helperCLCheckError(error)){ printErr("Failed to create OpenCL program!"); return nullptr;}
	error = clBuildProgram(program, deviceCount, &device, nullptr, nullptr, nullptr);
	//Program failed to build, probably syntax error
	if(!helperCLCheckError(error)){
		printErr("Failed to build OpenCL program!");
		// Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		// Allocate memory for the log
		char *log = (char *) malloc(log_size);
		// Get the log
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
		// Print the log
		printf("%s\n", log);
		free(log);
		return nullptr;
	}

	//Build kernel (can contain multiple programs), reference the function to use by name
    kernel = clCreateKernel(program, "saxpy_kernel", &error);
	if(!helperCLCheckError(error)){
		printErr("Failed to create OpenCL kernel!");
		switch(error){
			case CL_INVALID_PROGRAM:{
				printErr("Invalid program.");
			} break;
			case CL_INVALID_PROGRAM_EXECUTABLE:{
				printErr("Invalid program executable.");
			} break;
			case CL_INVALID_KERNEL_NAME:{
				printErr("Kernel name not found.");
			} break;
			case CL_INVALID_VALUE:{
				printErr("Kernel name is null.");
			} break;
			case CL_OUT_OF_HOST_MEMORY:{
				printErr("No memory.");
			} break;
		}
		return nullptr;}

    //Set function arguments for the kernel
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&aBuffer);
    if(!helperCLCheckError(error)){ printErr("Could not set aBuffer."); return nullptr; }
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&bBuffer);
    if(!helperCLCheckError(error)){ printErr("Could not set bBuffer."); return nullptr; }
    clSetKernelArg(kernel, 2, sizeof(float), (void*)&two);
    if(!helperCLCheckError(error)){ printErr("Could not set two."); return nullptr; }

	//Enqueue the kernel into the OpenCL device for execution
    size_t globalWorkItemSize = dataSize;				//the total size of 1 dimension of the work items
	size_t workDimension = 1; 							//The dimension of the workgroup (1 - 3 dimensions)
    size_t workGroupSize = 1 * sizeof(float); 			//The size of one work group

    //5. 	Enqueue commands
    //5.1.	Write host memory to device memory - ONLY IF CL_MEM_COPY_HOST_PTR IS NOT SPECIFIED
    //		- in this case, we used it above
	//5.2	Compute on device
    error = clEnqueueNDRangeKernel(commandQueue, kernel, workDimension, nullptr, &globalWorkItemSize, &workGroupSize, 0, nullptr, nullptr);
    if(!helperCLCheckError(error)){ printErr("Could not enqueue commands."); return nullptr; }
	//5.3   Read results back
	error = clEnqueueReadBuffer(commandQueue, bBuffer, CL_TRUE, 0, dataSize, finalData, 0, nullptr, nullptr);
    if(!helperCLCheckError(error)){ printErr("Could not read buffer.");  return nullptr; }else{ printSuccess("Finished!"); }

    printf("hello");
	for(unsigned int i = 0; i < dataItems; i++){
		printf("%d\n", finalData[i]); //segfault here
	}

    free(finalData);
    clFlush(commandQueue);
    clFinish(commandQueue);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);

    clReleaseMemObject(aBuffer);
    clReleaseMemObject(bBuffer);

	printSuccess("Done.");
}

void* Job::stitchHeightTerrains(){
    //TODO: image processing in OpenCL
    status = JOB_STATUS::WAIT_HEIGHT_STITCHING;
	return nullptr;
}

void* Job::parsePBF(const char* filePath){
	//Start timer
	clock_t start = clock(), diff;

	//Open file and get file size
	struct stat statbuf;

	if (stat(filePath, &statbuf) == -1) {
		printErr(strerror(errno));
		return nullptr;
	}

	if(statbuf.st_size == 0 || statbuf.st_size > MAX_BLOB_SIZE){
        if(jobNeedsLogging){ printErr("PBF file size invalid."); }
        return nullptr;
	}

	//TODO: Make object for PBF file so that it is guaranteed to free itself on destruction
	FILE* pbfFile = fopen(filePath, "r");
    char* pbfFileBuf = (char*)malloc(statbuf.st_size);

    //TODO: doesn't work
	if(fread(pbfFileBuf, statbuf.st_size, 1, pbfFile) != statbuf.st_size){
		if(jobNeedsLogging){ printErr("PBF file reading got interrupted or file is corrupt."); }
		free(pbfFileBuf);
		return nullptr;
	}

	fclose(pbfFile);

	OSMPBF::Blob blob;

    if(!blob.ParseFromArray(pbfFileBuf, statbuf.st_size)){
		if(jobNeedsLogging){ printErr("Could not parse PBF file."); }
		free(pbfFileBuf);
		return nullptr;
    }

    if(!blob.has_raw()){
        //TODO: what now? PBF file has been openend
    }else{
#ifndef ZLIB_H
	#error ZLIB header must be included for ZLIB-decompression
#else

#endif
    }

    free(pbfFileBuf);
	return nullptr;
	//NOTE: DO NOT RETURN UNTIL free() IS CALLED!
    //Read header

    //Print bounding box



	//Timer
	if(jobNeedsLogging){
		diff = clock() - start;
		int msec = diff * 1000 / CLOCKS_PER_SEC;
		printf("PBF time taken %d seconds %d milliseconds", msec/1000, msec%1000);
	}

	free(pbfFileBuf);
	return nullptr;
}

void Job::determineArea(Job::Area& originialArea)
{
	//TODO
	double north = originialArea.NorthBoundingCoordinate;
	double east = originialArea.EastBoundingCoordinate;
	double west = originialArea.WestBoundingCoordinate;
	double south = originialArea.SouthBoundingCoordinate;

	unsigned int paperWidthMM = originialArea.paperWidthMM;
	unsigned int paperHeightMM = originialArea.paperHeightMM;

	//TODO: something is whacky about these calculations. They produce wrong results
	//Normalize coordinates (too big, too low, flipped)
    if(east > 360.0f){ 		east = 360.0f - 	remainderf(east, 360.0f);}
    if(east < -360.0f){ 	east = -360.0f + 	remainderf(east, 360.0f);}
    if(west > 360.0f){ 		west = 360.0f - 	remainderf(west, 360.0f);}
    if(west < -360.0f){ 	west = 360.0f + 	remainderf(west, 360.0f);}
    //Overshoot error north south shouldn't happen, just in case
	if(north > 90.0f){ 		north = 90.0f - 	remainderf(north, 90.0f);}
	if(north < -90.0f){ 	north = -90.0f + 	remainderf(north, 90.0f);}
	if(south > 90.0f){ 		north = 90.0f - 	remainderf(north, 90.0f);}
	if(south < -90.0f){ 	north = -90.0f + 	remainderf(north, 90.0f);}

    if(east < west){ double temp = east; east = west; west = temp;}
    if(north < south){ double temp = north; north = south; south = temp;}

	//Project coordinates into meter, proj4
	//Fake hacky google projection, used only by coordinates
	//FIXME: this should go in the job also see appwindow header
	std::string epsg3857 = "+proj=latlong +datum=WGS84 \
							+to +proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 \
							+x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs";

	projPJ fakeEPSG3857 = pj_init_plus(epsg3857.c_str());


    WORLD_POS midpoint((east - west) / 2, (north - south) / 2, epsg3857.c_str());
    WORLD_POS northEast(east, north, epsg3857.c_str());
    WORLD_POS northWest(west, north, epsg3857.c_str());
    WORLD_POS southWest(west, south, epsg3857.c_str());
    WORLD_POS southEast(east, south, epsg3857.c_str());

	unsigned int minDPI = 300;

	double distance;
	//Oblique azimuthal equidistant (centered on arbitrary point)
	//+proj=aeqd +lat_0=3.810790196801946 +lon_0=-0.703125 +units=m
	std::string tempProjForMeasurement = "+proj=aeqd +lat_0=";
	tempProjForMeasurement += std::to_string(midpoint.x);
	tempProjForMeasurement += " +units=m +lon_0=";
	tempProjForMeasurement += std::to_string(midpoint.y);

	projPJ measure = pj_init_plus(tempProjForMeasurement.c_str());

	if(fakeEPSG3857==NULL || measure==NULL){
		printErr("Invalid projections given.");
		return;
	}

	pj_transform(fakeEPSG3857, measure, 1, 1, &midpoint.x, &midpoint.y, NULL );
	pj_transform(fakeEPSG3857, measure, 1, 1, &northEast.x, &northEast.y, NULL );
	pj_transform(fakeEPSG3857, measure, 1, 1, &northWest.x, &northWest.y, NULL );
	pj_transform(fakeEPSG3857, measure, 1, 1, &southWest.x, &southWest.y, NULL );
	pj_transform(fakeEPSG3857, measure, 1, 1, &southEast.x, &southEast.y, NULL );

	midpoint.x *= RAD_TO_DEG;
	midpoint.y *= RAD_TO_DEG;

	printf("\n%f\t%f\n", midpoint.x, midpoint.y);
	printf("\n%f\t%f\n", northEast.x, northEast.y);
	printf("\n%f\t%f\n", northWest.x, northWest.y);
	printf("\n%f\t%f\n", southWest.x, southWest.y);
	printf("\n%f\t%f\n", southEast.x, southEast.y);
}

/*------------------ OpenCL helper functions --------------------*/

std::string Job::helperCLGetPlatformName(cl_platform_id& id)
{
	size_t size = 0;
	clGetPlatformInfo (id, CL_PLATFORM_NAME, 0, nullptr, &size);

	char platformName[size];
	clGetPlatformInfo (id, CL_PLATFORM_NAME, size,
		const_cast<char*>(platformName), nullptr);

	std::string actualName(platformName);
	delete platformName;
	return actualName;
}

std::string Job::helperCLGetDeviceName(cl_device_id& id)
{
	size_t size = 0;
	clGetDeviceInfo (id, CL_DEVICE_NAME, 0, nullptr, &size);

	char deviceName[size];
	clGetDeviceInfo (id, CL_DEVICE_NAME, size, const_cast<char*>(deviceName), nullptr);

	std::string actualName(deviceName);
	delete deviceName;
	return actualName;
}

//Check OpenCL error
bool Job::helperCLCheckError(cl_int& errorNr){
	if (errorNr != CL_SUCCESS) {
		printf("OpenCL call failed with error %d.\n", errorNr);
		return false;
	}
	return true;
}
