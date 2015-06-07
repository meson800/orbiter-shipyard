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




//the toolbox class handles its own events and loads the files assigned to it 
const int MGUIET_TOOLBOX = irr::gui::EGUIET_COUNT + 1;


class CGUIToolBox :
	public irr::gui::IGUIElement
{
public:
	CGUIToolBox(std::string _name, core::rect<s32> rectangle, irr::gui::IGUIEnvironment* environment, irr::gui::IGUIElement* parent);
	~CGUIToolBox(void);

	virtual irr::gui::IGUIEnvironment* GetEnvironment() { return Environment; }
	virtual const irr::c8* getTypeName() const   { return "toolbox"; }
	virtual bool OnEvent(const SEvent& event);

	virtual bool addElement(ToolboxData *newElement);				//creates a toolbox entry from a VesselData pointer
	void removeCurrentElement();								//removes the element that was right clicked last

	virtual void draw();
	ToolboxData *checkCreateVessel();										//if called, returns pointer to vesseldata to be created. NULL if no vessel is scheduled for creation. Returns to NULL after calling.
	std::string getName();
	void saveToolBox(std::string subfolder);
	void deleteToolBoxFromDisk(std::string subfolder);

private:
	vector<ToolboxData*> entries;
//	IGUIElement* maneuverGUI;

	gui::IGUIScrollBar* ScrollBar;

	int width;
	int imgWidth;
	int scrollPos;

	UINT GetEntryUnderCursor(int x);										//returns the index of the entry over which the cursor currently hovers
	ToolboxData *vesselToCreate;
	UINT rightClickedElement;												//stores the element that was right clicked last
	std::string name;

};
