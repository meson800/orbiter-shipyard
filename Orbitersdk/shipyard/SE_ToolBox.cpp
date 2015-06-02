#include "GuiIdentifiers.h"
#include "SE_ToolBox.h"


CGUIToolBox::CGUIToolBox(std::string _name, core::rect<s32> rectangle, irr::gui::IGUIEnvironment* environment, irr::gui::IGUIElement* parent)
: IGUIElement((irr::gui::EGUI_ELEMENT_TYPE)MGUIET_TOOLBOX, environment, parent, -1, rectangle)
{
	
	name = _name;
	width = rectangle.getWidth();
	imgWidth = 164;
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

	//setVisible(false);
}



CGUIToolBox::~CGUIToolBox(void)
{
	if (ScrollBar)
		ScrollBar->drop();

}


bool CGUIToolBox::OnEvent(const SEvent& event)
{
	if (!isEnabled())
		return IGUIElement::OnEvent(event);

	switch (event.EventType)
	{
	case EET_GUI_EVENT:
		switch (event.GUIEvent.EventType)
		{
		case gui::EGET_SCROLL_BAR_CHANGED:
			if (event.GUIEvent.Caller == ScrollBar)
				return true;
			break;
		}
	case EET_MOUSE_INPUT_EVENT:
		if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN || event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
		{
			if (Environment->hasFocus(this) &&
				!AbsoluteClippingRect.isPointInside(core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y)))
			{
				Environment->removeFocus(this);
				return false;
			}

			Environment->setFocus(this);
		}
		if (event.MouseInput.Event == EMIE_LMOUSE_DOUBLE_CLICK)
		{
			UINT entryToCreate = GetEntryUnderCursor(event.MouseInput.X);
			if (entryToCreate < entries.size())
			{
				vesselToCreate = entries[entryToCreate];
			}

		}
		else if (event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
		//create context menu for editting toolbox
		{
			IGUIContextMenu *mnu = Environment->addContextMenu(rect<s32>(event.MouseInput.X, event.MouseInput.Y, event.MouseInput.X + 50, event.MouseInput.Y + 50), 0, TOOLBOXCONTEXT);
			mnu->addItem(L"add vessel");
			mnu->addItem(L"remove vessel");
			mnu->addItem(L"create new toolbox");
			rightClickedElement = GetEntryUnderCursor(event.MouseInput.X);
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
		scrollPos = double(ScrollBar->getPos()) / 100 * ((entries.size() * imgWidth) - AbsoluteRect.getWidth());
		bool bugme = true;
	}

	IGUISkin* skin = Environment->getSkin();
	video::IVideoDriver* driver = Environment->getVideoDriver();

	driver->draw2DRectangle(skin->getColor(EGDC_3D_FACE), AbsoluteRect);

	//drawing vessel images
	//first partly visible entry
	UINT firstEntry = scrollPos / imgWidth;
	//last partly visible entry
	UINT lastEntry = firstEntry + (AbsoluteRect.getWidth() / imgWidth + 1);
	int imgDrawPos = firstEntry * imgWidth - scrollPos;

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
		if (entries.size() * imgWidth > width)
		{
			ScrollBar->setVisible(true);
			ScrollBar->setEnabled(true);
		}

		return true;
	}
	return false;
}


ToolboxData *CGUIToolBox::checkCreateVessel()
{
	if (vesselToCreate != NULL)
	//function automatically resets after it's been checked
	{
		ToolboxData* retVessel = vesselToCreate;
		vesselToCreate = NULL;
		return retVessel;
	}
	return NULL;
}


void CGUIToolBox::removeCurrentElement()
//removes the element from the toolbox that was right-clicked last
{
	if (rightClickedElement != -1)
	{
		entries.erase(entries.begin() + rightClickedElement);
	}
	rightClickedElement = -1;

	if (entries.size() * imgWidth <= width)
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
	std::string toolboxPath = std::string(Helpers::workingDirectory + "/StackEditor/Toolboxes/" + subfolder + name + ".tbx");
	ofstream toolboxFile = ofstream(toolboxPath.c_str(), ios::out);
	for (UINT i = 0; i < entries.size(); ++i)
	{
		toolboxFile << entries[i]->configFileName << "\n";
	}
	toolboxFile.close();
}


UINT CGUIToolBox::GetEntryUnderCursor(int x)
//returns the index of the entry over which the cursor currently hovers. x is x-position of mouse cursor
{
	int effectiveCursorPos = scrollPos + x;		//position of the cursor on the toolbox overall (not just visible part)
	return effectiveCursorPos / imgWidth;		//calculating index of entry at this position
}

