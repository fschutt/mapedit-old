#include "jobmanager.h"
#include "style.h"

#include <stdio.h>
#include "tinyxml2/tinyxml2.h"
#include <pthread.h>
#include "renderer.h"
#include "map_defs.h"

JobManager::JobManager()
{
	GDALAllRegister();
}

JobManager::~JobManager()
{
	for(Job* j : jobs){
		delete j;
	}
}

JobManager& JobManager::getInstance()
{
	//Instatiated on first use, auto-deleted
    static JobManager instance;
	return instance;
}

/*---------------------------------------- HELPER FUNCTIONS ----------------------------------------*/

glm::vec3 hexToRGB(const char* hex)
{
	char red[3] = {hex[0], hex[1], '\0'};
	char green[3] = {hex[2], hex[3], '\0'};
	char blue[3] = {hex[4], hex[5], '\0'};

	unsigned int r = strtol(red, NULL, 16);
    unsigned int g = strtol(green, NULL, 16);
    unsigned int b = strtol(blue, NULL, 16);

	glm::vec3 color(r, g, b);
	return color;
}

inline time_t getSafeTime(){
	time_t currentTime = time(NULL);
	if (currentTime == ((time_t)-1)){
		printf("Failed to get time ...\n");
	}
	return currentTime;
}

//Checks for sub-nodes called "outline" and "fill" and sets attributes accordingly
void parseLayerStyle(Style::Layer* layer, tinyxml2::XMLElement* layerXMLElement)
{
	tinyxml2::XMLElement* outlineXMLHandle = layerXMLElement->FirstChildElement("outline");
	tinyxml2::XMLElement* fillXMLHandle = layerXMLElement->FirstChildElement("fil");

	if(outlineXMLHandle != nullptr){
		if(!outlineXMLHandle->Attribute("use", "no")){

			int widthPt;
			const char* colorHex;

			outlineXMLHandle->FirstChildElement("width-pt")->QueryIntText(&widthPt);
			colorHex = outlineXMLHandle->FirstChildElement("color-hex")->GetText();
			if(&widthPt != nullptr){
				layer->outline.widthPt = widthPt;
			}

			if(colorHex != nullptr){
				layer->outline.colorRGB = hexToRGB(colorHex);
			}

			if(outlineXMLHandle->Attribute("overprint", "yes")){
				layer->outline.overprint = true;
			}
		}
	}

    //set fill
	if(fillXMLHandle != nullptr){
		if(!(fillXMLHandle->Attribute("use", "no"))){
			///FILL
			if(fillXMLHandle->Attribute("overprint", "yes")){
				layer->fill.overprint = true;
			}
			const char* colorHex = fillXMLHandle->FirstChildElement("color-hex")->GetText();
			if(colorHex != nullptr && strlen(colorHex) > 0){
				layer->fill.colorRGB = hexToRGB(colorHex);
			}
		}
	}
}

//Parse all layer
void recursiveParseLayers(Style::LayerGroup* rootGroup, tinyxml2::XMLElement* rootLayer)
{
	tinyxml2::XMLElement* topLevelLayer = rootLayer->FirstChildElement();
	time_t currentTime = getSafeTime();
	unsigned short zOrder = -1; //this is on purpose: underflow to get the highest zOrder
	unsigned short layerID = 0;

	while(topLevelLayer != nullptr && zOrder > 0){
		if(!topLevelLayer->NoChildren() || strcmp(topLevelLayer->FirstChild()->Value(), "outline") || strcmp(topLevelLayer->FirstChild()->Value(), "fill")){
			//layer is a layer
			Style::Layer* l = new Style::Layer(layerID);
			l->zOrder = zOrder;
			l->creation = currentTime;
			l->name = topLevelLayer->Name();
			l->parentLayerGroup = rootGroup;
			parseLayerStyle(l, topLevelLayer);
			rootGroup->subLayers.push_back(l);
		}else{
			//layer is a group
			Style::LayerGroup* lg = new Style::LayerGroup(layerID);
			lg->creation = currentTime;
			lg->name = topLevelLayer->Name();
			lg->zOrder = zOrder;
			lg->parentLayerGroup = rootGroup;
			rootGroup->subLayerGroups.push_back(lg);
			recursiveParseLayers(lg, topLevelLayer);
		}

		layerID++; //the layer id does not change afterwards
		zOrder--;
		topLevelLayer = topLevelLayer->NextSiblingElement();
	}
}

//XML parsing function, it is a bit messy
JobManager::xmlParseError JobManager::addJob(const char* filePath)
{
	///FILE PARSING
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = doc.LoadFile(filePath);

	if(err != tinyxml2::XML_SUCCESS){
		doc.PrintError();
		return xmlParseError::JOB_RESOURCE_UNAVAILABLE;
	}

	///JOB [FATAL]
	tinyxml2::XMLElement* jobElement = doc.RootElement();
	if(jobElement == nullptr){return xmlParseError::JOB_NO_JOB;}
	Job* currentJob = new Job;

	///METADATA [FATAL]
	tinyxml2::XMLElement* metadata = jobElement->FirstChildElement("metadata");
	if(metadata == nullptr){delete currentJob; return xmlParseError::JOB_NO_METADATA;}

	///VERSION [NONFATAL]
	tinyxml2::XMLElement* version = metadata->FirstChildElement("version");
	if(version == nullptr){delete currentJob; return xmlParseError::JOB_NO_VERSION;}
	version->QueryIntText(&currentJob->programVersion);
	int programVersion = ((MAJOR_VER * 100) + (MINOR_VER * 10) + PATCH_VER);
	if(currentJob->programVersion < programVersion){delete currentJob; return xmlParseError::JOB_VERSION_MISMATCH;}

	///REFFERER
	tinyxml2::XMLElement* referrer = metadata->FirstChildElement("referrer");
	if(referrer == nullptr){delete currentJob; return xmlParseError::JOB_NO_REFERRER;}
	referrer->QueryIntText(&currentJob->referrer);

	///AUTH
	tinyxml2::XMLElement* auth = metadata->FirstChildElement("auth");
	if(auth == nullptr){delete currentJob; return xmlParseError::JOB_NO_AUTH_CODE;}
	currentJob->authCode = auth->GetText();

	///GUID
	tinyxml2::XMLElement* GUID = metadata->FirstChildElement("guid");
	if(GUID == nullptr){delete currentJob; return xmlParseError::JOB_NO_GUID;}
	currentJob->GUID = GUID->GetText();

	///STYLE
	tinyxml2::XMLElement* style = metadata->FirstChildElement("style");
	if(style == nullptr){delete currentJob; return xmlParseError::JOB_NO_STYLE;}

	///RELIEF [NON-FATAL]
	tinyxml2::XMLElement* relief = style->FirstChildElement("relief");
	if(relief != nullptr && !relief->Attribute("use", "no")){
		currentJob->style.reliefDefinition = new Style::ReliefDefinition;
		if(relief->Attribute("shading", "topographic")){
			currentJob->style.reliefDefinition->shading = Style::ReliefDefinition::SHADING::TOPOGRAPHIC;
		}else if(relief->Attribute("shading", "greyscale")){
			currentJob->style.reliefDefinition->shading = Style::ReliefDefinition::SHADING::MONOCHROME;
		}else if(relief->Attribute("shading", "hillshading")){
			currentJob->style.reliefDefinition->shading = Style::ReliefDefinition::SHADING::HILLSHADING;
		}
	}

	///CONTOURS
	tinyxml2::XMLElement* contours = style->FirstChildElement("contours");
	while(contours != nullptr && !contours->Attribute("use", "no")){
		Style::ContoursDefinition* currentContours = new Style::ContoursDefinition;

		tinyxml2::XMLElement* outline = contours->FirstChildElement("outline");
		if(outline == nullptr && !outline->Attribute("overprint", "no")){ delete currentContours; break; }
		currentContours->outline.overprint = true;

		float contoursOutlineWidth;
		const char* colorHex;
		tinyxml2::XMLElement* contoursWidth = outline->FirstChildElement("width-pt");
		if(contoursWidth == nullptr){ delete currentContours; break; }
		if(contoursWidth->QueryFloatText(&contoursOutlineWidth) != tinyxml2::XML_SUCCESS){ break;}
		currentContours->outline.widthPt = contoursOutlineWidth;

		tinyxml2::XMLElement* contoursColorHex = outline->FirstChildElement("color-hex");
		if(contoursColorHex == nullptr){ delete currentContours; break; }
		currentContours->outline.colorRGB = hexToRGB(contoursColorHex->GetText());

		tinyxml2::XMLElement* minor = contours->FirstChildElement("minor");
		if(minor == nullptr){ delete currentContours; break; }
		minor->QueryIntText(&currentContours->minorContours);

		tinyxml2::XMLElement* major = contours->FirstChildElement("major");
		if(major == nullptr){ delete currentContours; break; }
		major->QueryIntText(&currentContours->majorContours);

		currentJob->style.contoursDefinition = currentContours;
		break;
	}

	///LAYERS
	tinyxml2::XMLElement* layers = style->FirstChildElement("layers");
	if(layers == nullptr){ delete currentJob; return xmlParseError::JOB_NO_LAYERS; }
	recursiveParseLayers(currentJob->style.layers, layers);

	///AREALIST [FATAL]
	tinyxml2::XMLElement* arealist = jobElement->FirstChildElement("arealist");
	if(arealist == nullptr){ delete currentJob; return xmlParseError::JOB_NO_AREAS; }

	///AREAS
	tinyxml2::XMLElement* area = arealist->FirstChildElement();
	while( area != nullptr){
		Job::Area currentArea;

		const char* scaleOrFormatFixed = area->Attribute("fixed");
		if(strcmp(scaleOrFormatFixed, "scale")){
			currentArea.fixedScale = true;
		}

		currentArea.projection = area->Attribute("proj4");
		area->QueryAttribute("width-paper-mm", &currentArea.paperWidthMM);
		area->QueryAttribute("height-paper-mm", &currentArea.paperHeightMM);
		area->QueryAttribute("scale", &currentArea.scale);
		area->FirstChildElement("north")->QueryFloatText(&currentArea.NorthBoundingCoordinate);
		area->FirstChildElement("west")->QueryFloatText(&currentArea.WestBoundingCoordinate);
		area->FirstChildElement("south")->QueryFloatText(&currentArea.SouthBoundingCoordinate);
		area->FirstChildElement("east")->QueryFloatText(&currentArea.EastBoundingCoordinate);

		if(strlen(currentArea.projection) <= 0){ break; }
		if(&currentArea.paperWidthMM == nullptr){ break; }
		if(&currentArea.paperHeightMM == nullptr){ break; }
		if(&currentArea.scale == nullptr){ break; }
		if(&currentArea.NorthBoundingCoordinate == nullptr){ break; }
		if(&currentArea.WestBoundingCoordinate == nullptr){ break; }
		if(&currentArea.SouthBoundingCoordinate == nullptr){ break; }
		if(&currentArea.EastBoundingCoordinate == nullptr){ break; }

		currentJob->arealist.push_back(currentArea);
		area = area->NextSiblingElement();
	}

	jobs.push_back(currentJob);
	return xmlParseError::JOB_NO_ERROR;
}

void* JobManager::startNextJob(void* params)
{
	int threadReturnCode = 0;
	std::vector<Job*>* allJobs = (std::vector<Job*>*)params;
	Job* jobToTake;

	//get next free job
	for(Job* j : *allJobs){
		if(pthread_mutex_trylock(&j->mutex) != EBUSY){
			jobToTake = j;
			break;
		}
	}

	//TODO: check system resources

	bool systemHasResources = true;

	if(jobToTake != nullptr)
	{
		//there might be no jobs to take anymore
		if(systemHasResources)
		{
			Renderer r(&jobToTake->style);

			ThreadParameters params;
			params.renderer = &r;

			for(Job::Area area : jobToTake->arealist){
				if(pthread_mutex_trylock(&area.mutex)){

					params.area = area;
					int err;
					pthread_t drawingThread;
					err = pthread_create(&drawingThread, NULL, &Renderer::draw, (void*)&params);
					if(err){ printf("Unable to create rendering thread: %d\n", err);}

					//do other stuff

					pthread_join(drawingThread, NULL);
				}

				//fetch data overpass api (new thread)

				//calculate height contours, stitch images	(new thread)

				//wait for overpass thread
				//filter OSM data (own thread)
				//wait for filter thread
				//generalize OSM data (own thread)
				//calculate font positions (own thread)
				//wait for generalizing thread
				//wait for font positioning thread
				//wait for height contours thread
			}
		}else{
			//suspend until resources are free
		}
	}else{
		//all jobs have been taken
        threadReturnCode = -1;
	}

    pthread_mutex_unlock(&jobToTake->mutex);
	pthread_exit((void*)threadReturnCode);
}


