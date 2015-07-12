#include "OrbiterDockingPort.h"

DockingPortStatus OrbiterDockingPort::returnStatus()
{
    DockingPortStatus output;
    output.docked = docked;
    output.dockedTo = dockedTo;
    return output;
}