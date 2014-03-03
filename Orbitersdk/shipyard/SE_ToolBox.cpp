#include "GuiIdentifiers.h"
#include "SE_ToolBox.h"

CGUIToolBox::CGUIToolBox(core::rect<s32> rectangle, irr::gui::IGUIEnvironment* environment, irr::gui::IGUIElement* parent)
: IGUIElement((irr::gui::EGUI_ELEMENT_TYPE)MGUIET_TOOLBOX, environment, parent, -1, rectangle)
{
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
		if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
		{
			if (Environment->hasFocus(this) &&
				!AbsoluteClippingRect.isPointInside(core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y)))
			{
				Environment->removeFocus(this);
				return false;
			}

			Environment->setFocus(this);
		}
		else if (event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
		{
			IGUIContextMenu *mnu = Environment->addContextMenu(rect<s32>(event.MouseInput.X, event.MouseInput.Y, event.MouseInput.X + 50, event.MouseInput.Y + 50), this, TOOLBOX);
			mnu->addItem(L"add vessel");
			mnu->addItem(L"remove vessel");
		}
	}
	return IGUIElement::OnEvent(event);
}


void CGUIToolBox::draw()
{
	if (!IsVisible)
	{
		return;
	}

	if (ScrollBar->isVisible())
	{
		scrollPos = ScrollBar->getPos();
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
		if (entries[i]->vesselImg != NULL)
		{
			driver->draw2DImage(entries[i]->vesselImg, vector2d<s32>(AbsoluteRect.UpperLeftCorner.X + imgDrawPos, AbsoluteRect.UpperLeftCorner.Y + 2));
			imgDrawPos += (imgWidth);
		}
	}

	IGUIElement::draw();

}


bool CGUIToolBox::addElement(VesselData *newElement)
//adds a ToolBox Entry from the passed VesselData. Returns false if pointer is NULL
{
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