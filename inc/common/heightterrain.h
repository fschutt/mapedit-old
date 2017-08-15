#pragma once

#include "stb_image.h"
#include <vector>
#include <time.h>
#include "job.h"

struct HeightTerrain
{
	/*
		SRTM data are distributed in two levels: SRTM1 (for the U.S. and its territories and possessions)
		with data sampled at one arc-second intervals in latitude and longitude, and SRTM3 (for the world) sampled at three arc-seconds.
		Data are divided into one by one degree latitude and longitude tiles in "geographic" projection, which is to say a raster
		presentation with equal intervals of latitude and longitude in no projection at all but easy to manipulate and mosaic.
		File names refer to the latitude and longitude of the lower left corner of the tile - e.g. N37W105 has its lower left corner
		at 37 degrees north latitude and 105 degrees west longitude. To be more exact, these coordinates refer to the geometric
		center of the lower left pixel, which in the case of SRTM3 data will be about 90 meters in extent.
		Height files have the extension .HGT and are signed two byte integers. The bytes are in Motorola "big-endian"
		order with the most significant byte first, directly readable by systems such as Sun SPARC, Silicon Graphics and
		Macintosh computers using Power PC processors. DEC Alpha, most PCs and Macintosh computers built after 2006 use
		Intel ("little-endian") order so some byte-swapping may be necessary. Heights are in meters referenced to the WGS84/EGM96
		geoid. Data voids are assigned the value -32768.

		https://en.wikipedia.org/wiki/Shuttle_Radar_Topography_Mission
		http://www2.jpl.nasa.gov/srtm/

		Server with SRTM Version 3 images can be contacted over HTTPS and is available at:
		https://e4ftl01.cr.usgs.gov/SRTM
		/SRTMGL3.003						SRTM 3-degree
		/SRTMGL30.003

		SRTM Files have the naming convention:
		(SRTM 3 deg):

		(SRTM 30 deg):

		//------------------------------------------------------------------
	*/

	HeightTerrain();
	virtual ~HeightTerrain();

	enum class DEM_OUTPUT_FORMAT{
		PNG,
		TGA
	};

	DEM_OUTPUT_FORMAT outputFormat;

	enum class DEM_FIDELITY{
		THREE_DEGREES,
		THIRTY_DEGREES
	};

	DEM_FIDELITY demFidelity;

	//NOTE(felix): Update this when newer versions on release of SRTM
	//Copyright SRTM 3 deg Version 3.0
	constexpr static const char* srtm3DegCopyright = "(C) 2013 NASA JPL. - NASA SRTM Global 3 arc second - NASA LP DAAC. \nRetrieved from https://doi.org/10.5067/MEaSUREs/SRTM/SRTMGL3.003";
	//Copyright SRTM 30 deg Version 2.0
	constexpr static const char* srtm30DegCopyright = "(C) 2013 NASA JPL. - NASA SRTM Global 30 arc second - NASA LP DAAC. \nRetrieved from https://doi:10.5067/MEaSUREs/SRTM/SRTMGL30.002.";

	//Geoid used
	const char* geoidRef = "WGS84/EGM96";

	//Value used for filling void datasets
	int fillValue;

	//Filename and real name
	std::vector<const char*> filesNeededForDEM;

	//Selects the file names to load for one job
	//If the job needs 30 or 3-degree files should be determined in the job
	void determineFiles(Job::Area& area);

	//Storage on a G2 instance is limited to 60GB so it makes sense to decompress the images we need
	void decompressZIP();
	void hgtToPNG();

	//interval = "50 m" majorContourMultiplier = "5", so every 250 m a new major height contour is generated
	void generateHeightContours(unsigned int interval,
								unsigned int minorContourMultiplier = 1,
								unsigned int majorContourMultiplier = 5,
								unsigned int resolutionInMeters = 30,
								bool smoothContours = true);

	bool loadFromHGTZIP(const char* filePath);
	bool renderToLocalFile(const char* filePath);
};
