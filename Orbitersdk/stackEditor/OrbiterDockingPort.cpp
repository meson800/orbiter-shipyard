//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
#include "OrbiterDockingPort.h"

DockingPortStatus OrbiterDockingPort::returnStatus()
{
    DockingPortStatus output;
    output.docked = docked;
    output.dockedTo = dockedTo;
    return output;
}
