#pragma once
#include "time.h"
#include <vector>
#include <map>

#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "fontface.h"

struct HeightTerrain;

struct Style
{
	struct LayerGroup;

	Style();
	~Style();

	///STRUCTS
	///See sample.xml for structure

	/*------ GENERIC --------*/
	struct Symbol{
		unsigned int ID;
        const char* name;
		const char* svg;
		unsigned int widthTotal;
		unsigned int heightTotal;
	};

	struct FillPattern{
        std::vector<Symbol> elements;
        double radomization;
        double elementSizePt;
        double offsetHorz;
        double offsetVert;
	};

	struct Outline
	{
        int widthPt;
        bool overprint = false;
        glm::vec3 colorRGB;
		std::map<double, double> pattern; //TODO
	};

	struct Fill
	{
		FillPattern pattern;
		glm::vec3 colorRGB;
		bool overprint = false;
	};

	struct Layer{
		Layer(const unsigned short id) : ID(id){ }
		~Layer(){ }
		const char* name;
		const unsigned short ID;
		unsigned short zOrder;
		time_t creation;
		LayerGroup* parentLayerGroup = nullptr;

		std::vector<time_t> modificationTimestamps;

		Outline outline;
		Fill fill;
	};

	struct LayerGroup{
		LayerGroup(const unsigned short id) : ID(id) { }
		~LayerGroup(){ }
		const char* name;
		const unsigned short ID;
		unsigned short zOrder;
		time_t creation;
		LayerGroup* parentLayerGroup = nullptr;

		std::vector<time_t> modificationTimestamps;
		std::vector<Layer*> subLayers;
		std::vector<LayerGroup*> subLayerGroups;
	};

	/*----- END GENERIC ------*/

	struct ContoursDefinition
	{
		int minorContours;
		int majorContours;
		Outline outline;
	};

	struct ReliefDefinition
	{
		HeightTerrain* relief = nullptr;
		enum class SHADING {
			TOPOGRAPHIC,
			MONOCHROME,
			HILLSHADING
		} shading = SHADING::MONOCHROME;
	};

	struct GPXTrack{
		const char* gpxSource;
        const char* epsgCoordSystem;
	};

	ReliefDefinition* reliefDefinition;
	GPXTrack* gpxTrack;
	ContoursDefinition* contoursDefinition;

	LayerGroup* layers; //has one element by default, see constructor
	std::map<FontFace, std::vector<Layer*>> fonts;
	std::vector<glm::vec3> colors;
    std::map<const char*, Layer*> sqlQueries;

private:
	//helper functions for recursive destruction
	void deleteLayerGroup(LayerGroup* lg);
	void deleteAllLayersAndGroups();
};
