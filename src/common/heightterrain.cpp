#include "heightterrain.h"
#include "print_log_console.h"
#ifdef SERVER_BUILD
	#include "server.h"
#endif

#include <zip.h>
#include <string>
#include <math.h>
#include "map_defs.h"

HeightTerrain::HeightTerrain()
{
	//ctor
}

HeightTerrain::~HeightTerrain()
{
	//dtor
}

void HeightTerrain::decompressZIP()
{

	if(filesNeededForDEM.empty()){
		printErr("No files specified for DEM creation.");
	}

	for(const char* file : filesNeededForDEM)
	{
		std::string fileName(file);

		switch(demFidelity){
			case DEM_FIDELITY::THREE_DEGREES:{
                fileName += Server::getInstance().heightTerrainFolderSRTM3Deg;
			} break;
			case DEM_FIDELITY::THIRTY_DEGREES:{
                fileName += Server::getInstance().heightTerrainFolderSRTM30Deg;
			} break;
		}

		fileName += ".hgt.zip";

		//Open the ZIP archive
		int err = 0;
		zip *z = zip_open(fileName.c_str(), 0, &err);

		//Search for the file of given name
		const char *name = "file.hgt";
		struct zip_stat st;
		zip_stat_init(&st);
		zip_stat(z, name, 0, &st);

		//Alloc memory for its uncompressed contents
		char* contents = new char[st.size];

		//Read the compressed file
		zip_file *f = zip_fopen(z, name, 0);
		zip_fread(f, contents, st.size);
		zip_fclose(f);

		//And close the archive
		zip_close(z);

		//Do something with the contents
		//delete allocated memory
		delete[] contents;

	}


}

void HeightTerrain::hgtToPNG()
{
	printf("HGT to PNG");
}

void HeightTerrain::determineFiles(Job::Area& area)
{
    //char* tempProjForMeasurement = "+proj=eqdc +lat_1=15.640193445274836 +lat_2=48.26413103077511 +lon_0=64.6875";
	//We need to determine the best possible projection while respection the users input values in terms of

	//IMPROVE(felix): find projection with least distortion in a programmatic fashion
	//TODO: Implement this on website

	//Whole world:
	//Continent < 30 deg north south:	Mercator
	//+proj=merc +lon_0=11.25

	//Continent > 30deg north south:	Lambert conformal conic
	//+proj=lcc +lat_1=38.612715526299795 +lat_2=68.04903043973434 +lon_0=21.09375

	//Few countries:	Albers equal-area conic
	//Smaller: UTM
	//char* destProjection = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";

/*
	projPJ source = pj_init_plus(epsg3857);
	projPJ target = pj_init_plus(destProjection);

	if(source==NULL || target==NULL){
		printErr("Invalid projections given.")
		return;
	}

	//Transform point
	//No error checking because of bad API and even if, what could we do about it?
	pj_transform(source, target, 1, 1, &x, &y, NULL );

	x *= RAD_TO_DEG;
	y *= RAD_TO_DEG;

	printf("%f\t%f\n", x, y);
    //Determine if we need 3 or 30 degree resolution based on paper mm
    unsigned int pixelW = paperWidthMM * minDPI;
    unsigned int pixelH = paperHeightMM * minDPI;

    //If 30 degree resolution is needed, we are rather far zoomed out, determine best projection
*/

}

//interval = "50 m" majorContourMultiplier = "5", so every 250 m a new major height contour is generated
void HeightTerrain::generateHeightContours(unsigned int interval,
							unsigned int minorContourMultiplier,
							unsigned int majorContourMultiplier,
							unsigned int resolutionInMeters,
							bool smoothContours)
{
    printf("Generating height contours ...");
	return;
}

bool HeightTerrain::loadFromHGTZIP(const char* filePath)
{
    printf("Loading .hgt file ...");
	return true;
}

bool HeightTerrain::renderToLocalFile(const char* filePath)
{
    printf("Exporting to file ...");
	return true;
}
