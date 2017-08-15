#include "style.h"

Style::Style()
{
	//ctor
	Style::LayerGroup* defaultGroup = new LayerGroup(0);
	layers = defaultGroup;
}

void Style::deleteLayerGroup(LayerGroup* lg)
{
	//lg shouldn't be nullptr, but doesn't hurt to check
	if(lg != nullptr){
		for(LayerGroup* lgp : lg->subLayerGroups){
			deleteLayerGroup(lgp);
		}

		for(Layer* l : lg->subLayers){
			delete l;
		}

		delete lg;
	}
}

Style::~Style()
{
	if(reliefDefinition != nullptr){
		delete reliefDefinition;
	}

	if(gpxTrack != nullptr){
		delete gpxTrack;
	}

	if(contoursDefinition != nullptr){
		delete contoursDefinition;
	}

	//does not throw
	deleteLayerGroup(layers);
}
