#pragma once
#include "Common.h"
#include "DataManager.h"
//#include "CGUIScrollBar.h"

using namespace irr;
using namespace irr::core;
using namespace irr::video;
using namespace irr::gui;
using namespace irr::scene;
using namespace std;

//the toolbox class handles its own GUI and loads the files assigned to it 
const int MGUIET_TOOLBOX = irr::gui::EGUIET_COUNT + 1;


class CGUIToolBox :
	public irr::gui::IGUIElement
{
public:
	CGUIToolBox(core::rect<s32> rectangle, irr::gui::IGUIEnvironment* environment, irr::gui::IGUIElement* parent);
	~CGUIToolBox(void);

	virtual irr::gui::IGUIEnvironment* GetEnvironment() { return Environment; }
	virtual const irr::c8* getTypeName() const   { return "toolbox"; }
	virtual bool OnEvent(const SEvent& event);

	virtual bool addElement(VesselData *newElement);				//creates a toolbox entry from a VesselData pointer

	virtual void draw();
private:
	vector<VesselData*> entries;
//	IGUIElement* maneuverGUI;

	gui::IGUIScrollBar* ScrollBar;

	int width;
	int imgWidth;
	int scrollPos;
};
