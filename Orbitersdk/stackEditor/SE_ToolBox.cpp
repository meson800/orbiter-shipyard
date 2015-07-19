//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
#include "GuiIdentifiers.h"
#include "SE_ToolBox.h"


CGUIToolBox::CGUIToolBox(std::string _name, core::rect<s32> rectangle, irr::gui::IGUIEnvironment* environment, irr::gui::IGUIElement* parent)
: IGUIElement((irr::gui::EGUI_ELEMENT_TYPE)MGUIET_TOOLBOX, environment, parent, -1, rectangle)
{
	
	name = _name;
	width = rectangle.getWidth();
	imgWidth = 130;
	scrollPos = 0;
	
	IGUISkin* skin = Environment->getSkin();
	const s32 s = skin->getSize(EGDS_SCROLLBAR_SIZE);

	ScrollBar = environment->addScrollBar(true,
		core::rect<s32>(0, RelativeRect.getHeight() - s, RelativeRect.getWidth(), RelativeRect.getHeight()),
		this);

	ScrollBar->setSubElement(true);
	ScrollBar->setTabStop(false);
	ScrollBar->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
	ScrollBar->setVisible(false);
	ScrollBar->setPos(0);

	vesselToCreate = NULL;
	rightClickedElement = -1;
	hasbeenedited = false;
	lasthovered = -1;
}



CGUIToolBox::~CGUIToolBox(void)
{
	if (ScrollBar)
		ScrollBar->drop();

}


bool CGUIToolBox::OnEvent(const SEvent& event)
{
	if (!isEnabled() || !Environment->hasFocus(this))
		return IGUIElement::OnEvent(event);

	switch (event.EventType)
	{
	case EET_GUI_EVENT:
		switch (event.GUIEvent.EventType)
		{
		case gui::EGET_SCROLL_BAR_CHANGED:
			if (event.GUIEvent.Caller == ScrollBar)
				return true;
		case gui::EGET_ELEMENT_FOCUS_LOST:
			if (event.GUIEvent.Caller == this)
			{
				if (lasthovered != -1 && entries[lasthovered]->imsData != NULL)
				{
					entries[lasthovered]->imsData->setVisible(false);
					lasthovered = -1;
				}
			}
			break;
		}
	case EET_MOUSE_INPUT_EVENT:
		if (event.MouseInput.Event == EMIE_LMOUSE_DOUBLE_CLICK)
		{
			//due to a quirk of irrlicht, doubleclicks get handled as a separate event even if the individual clicks already produced an event
			//this makes it neccessary to check if the cursor is over the scrollbar, otherwise fast-clicking on the bar will spawn vessels
			if (!ScrollBar->isVisible() || event.MouseInput.Y < ScrollBar->getAbsoluteClippingRect().UpperLeftCorner.Y)
			{
				UINT entryToCreate = GetEntryUnderCursor(event.MouseInput.X);
				if (entryToCreate < entries.size())
				{
					vesselToCreate = entries[entryToCreate];
				}
			}
		}
		else if (event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
		//create context menu for editing toolbox
		{
			IGUIContextMenu *mnu = Environment->addContextMenu(rect<s32>(event.MouseInput.X, event.MouseInput.Y, event.MouseInput.X + 50, event.MouseInput.Y + 50), 0, TOOLBOXCONTEXT);
			mnu->addItem(L"add vessel");
			mnu->addItem(L"remove vessel");
			mnu->addItem(L"create new toolbox");
			mnu->addItem(L"delete toolbox");
			rightClickedElement = GetEntryUnderCursor(event.MouseInput.X);
		}
		else if (event.MouseInput.Event == EMIE_MOUSE_MOVED)
		{
			int elementhovered = GetEntryUnderCursor(event.MouseInput.X);
			if (elementhovered != lasthovered)
			//new element hovered, update visible ims data
			{
				//hide old table
				if (lasthovered != -1 && entries[lasthovered]->imsData != NULL)
				{
					entries[lasthovered]->imsData->setVisible(false);
				}
			
				//show new table
				if (elementhovered != -1 && entries[elementhovered]->imsData != NULL)
				{
					entries[elementhovered]->imsData->setVisible(true);
				}
			}
			lasthovered = elementhovered;
		}
	}
	return IGUIElement::OnEvent(event);
}


void CGUIToolBox::draw()
{
	if (!isVisible())
	{
		return;
	}

	if (ScrollBar->isVisible())
	{
		scrollPos = (int)double(ScrollBar->getPos()) / 100 * ((entries.size() * imgWidth) - AbsoluteRect.getWidth());
	}

	IGUISkin* skin = Environment->getSkin();
	video::IVideoDriver* driver = Environment->getVideoDriver();
	
	//draw background
	driver->draw2DRectangle(skin->getColor(EGDC_3D_FACE), AbsoluteRect);

	//drawing vessel images
	//first partly visible entry (because of scrollposition)
	UINT firstEntry = scrollPos / imgWidth;
	//last partly visible entry
	UINT lastEntry = firstEntry + (AbsoluteRect.getWidth() / imgWidth + 1);
	int imgDrawPos = 2 + firstEntry * imgWidth - scrollPos;

	for (UINT i = firstEntry; i < entries.size() && i < lastEntry; i++)
	{
		if (entries[i]->toolboxImage != NULL)
		{
			driver->draw2DImage(entries[i]->toolboxImage, vector2d<s32>(AbsoluteRect.UpperLeftCorner.X + imgDrawPos, AbsoluteRect.UpperLeftCorner.Y + 2));
			imgDrawPos += (imgWidth);
		}
	}

	IGUIElement::draw();

}


bool CGUIToolBox::addElement(ToolboxData *newElement)
//adds a ToolBox Entry from the passed VesselData. Returns false if pointer is NULL
{
	rightClickedElement = -1;
	if (newElement != NULL)
	{
		entries.push_back(newElement);
		if (((int)entries.size()) * imgWidth > width)
		{
			ScrollBar->setVisible(true);
			ScrollBar->setEnabled(true);
		}
		hasbeenedited = true;
		return true;
	}
	return false;
}


ToolboxData *CGUIToolBox::checkCreateVessel()
{
	if (vesselToCreate != NULL)
	//function automatically resets after it's been checked
	{
		ToolboxData *createvessel = vesselToCreate;
		vesselToCreate = NULL;
		return createvessel;
	}
	return NULL;
}

void CGUIToolBox::removeCurrentElement()
//removes the element from the toolbox that was right-clicked last
{
	if (rightClickedElement != -1)
	{
		entries.erase(entries.begin() + rightClickedElement);
		hasbeenedited = true;
	}
	rightClickedElement = -1;

	if (((int)entries.size()) * imgWidth <= width)
	{
		ScrollBar->setVisible(false);
		ScrollBar->setEnabled(false);
		scrollPos = 0;
	}
}

std::string CGUIToolBox::getName()
{
	return name;
}

void CGUIToolBox::saveToolBox(std::string subfolder)
{
	//only save the toolbox if it has been edited. Otherwise modules that could not be loaded will be removed 
	//from the toolbox after first load, which is not desirable as it would complicate fixing the problem
	//which is undesirable
	if (hasbeenedited)
	{
		std::string toolboxPath = std::string(Helpers::workingDirectory + "/StackEditor/Toolboxes/" + subfolder + name + ".tbx");
		ofstream toolboxFile = ofstream(toolboxPath.c_str(), ios::out);
		for (UINT i = 0; i < entries.size(); ++i)
		{
			toolboxFile << entries[i]->configFileName << "\n";
		}
		toolboxFile.close();
	}
}

//delete the toolbox file from harddisk
void CGUIToolBox::deleteToolBoxFromDisk(std::string subfolder)
{
	std::string toolboxPath = std::string(Helpers::workingDirectory + "/StackEditor/Toolboxes/" + subfolder + name + ".tbx");
	std::remove((const char*)toolboxPath.c_str());
}


int CGUIToolBox::GetEntryUnderCursor(int x)
//returns the index of the entry over which the cursor currently hovers. x is x-position of mouse cursor
//returns -1 if there's nothing under the cursor
{
	int effectiveCursorPos = scrollPos + x;		//position of the cursor on the toolbox overall (not just visible part)
	int idx = effectiveCursorPos / imgWidth;		//calculating index of entry at this position
	if (idx >= (int)entries.size())
	{
		idx = -1;
	}
	return idx;
}

void CGUIToolBox::finishedLoading()
//tells the toolbox that it has finished loading. Otherwise the loading itself would count as editing.
{
	hasbeenedited = false;
}
