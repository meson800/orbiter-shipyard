Orbiter Stack-Editor
================
Co-created by meson800 and jedidia

What is it?
------------
StackEditor is both a standalone and in-Orbiter addon that allows for
click and drag construction of larger ships/space stations, IMS-style, out of smaller
Orbiter vessels.  StackEditor can also import and export these constructions
to and from Orbiter.

What isn't it?
---------------
StackEditor is not Orbiter Shipyard or IMS, which create one Orbiter vessel out of
other sub-vessels.  StackEditor can certainly help by exporting all of the vessels
docked together in Orbiter, but does not have any integration features like IMS.

Installation
------------
Use standard Orbiter addon installation.

Unzip a zip downloaded from Orbit Hangar or from
[Github](https://github.com/meson800/orbiter-stackEditor/releases/tag/v1.0.0)
directly into your Orbiter directory.

Configuration
-------------
StackEditor is configured with its own config file located at:
ORBITER_ROOT/StackEditor/StackEditor.cfg

The config file contains documentation on how to set settings.

You can change screen resolution, default toolbox set, and log level in the config file.

StackEditor comes with two toolbox sets, Default, which contains a small amount of IMS toolboxes,
and IMS, a more full-featured toolbox set.


How do I use it?
----------------
To use in standalone mode, run StackEditor_standalone.exe from the root Orbiter
folder.  The only restriction in standalone mode is that you cannot import/export to Orbiter.

To use in linked mode inside Orbiter, go F4->Custom->StackEditor.

When StackEditor loads, you will see four main sections of the GUI.
Most of the window will be taken up by the main screen.  This is where you manipulate all nodes in the session.

In the upper left is the toolbox list.  StackEditor comes with a default set of IMS toolboxes
to get you started.

Below the toolbox list is the session manipulation buttons.  Save/save as saves the current session,
load loads the current session.

In the lower portion is the currently selected toolbox.  Double click an item to add it into
your current session.

Left-click selects a stack (a node and all nodes docked to it).  If you hover the cursor over a docking
port (blue sphere) of another stack/vessel, their docking ports will snap together.
Clicking again will dock the two stacks.

Pressing tab with a stack selected allows you to undock a stack.  Tab will show currently docked ports on the
selected vessel.  Simply click a docking port to undock that, splitting the stack.

When using StackEditor in linked mode, you can export your current stack to Orbiter by selecting it,
pressing ctrl-o and entering a name for the stack. Pay attention to deactivate the OrbiterSound module before doing this.
The resulting dock message spam typical for larger stacks will most probably crash Orbiter.

You can also import a stack that exists inside Orbiter into StackEditor. Make sure a vessel from the stack
is currently the focus vessel in Orbiter, and press ctr-i. If you make changes to an existing stack and
re-export it, StackEditor will modify the actual stack in Orbiter, instead of just spawning it anew.
Only vessels that were not previously in the stack will be newly created. Vessels that were removed from the stack,
however, will not be deleted in Orbiter. They'll just float off into space.
These rules also apply if you re-export a previously exported stack, provided the scenario is still the same.

Keymap
------
Left click         - Selects/deselects a stack  
Shift-left click   - Quick-adds the last created node  
Right-click held   - rotate Camera  
Niddle mouse button or Control-mouse move - Camera move  
Middle-mouse wheel - Camera zoom  
Tab                - Switches to undocking view with selected stack  
ASDWQE             - Rotation keys for the selected stack  
Control-o          - Exports the currently selected stack to Orbiter (if in linked mode)  
Control-i          - Imports the current Orbiter focus vessel and all connected vessels (if in linked mode)  
Control-c          - Creates a copy of the currently selected stack  
Delete             - Deletes the currently selected stack  
Control-z          - Undo  
Control-y          - Redo  

Toolboxes
---------
You can add or delete nodes from each toolbox, or add and delete toolboxes.

To add a toolbox, right-click in the toolbox item area(lower part) and select "Create new toolbox"

To delete a toolbox, right-click in the toolbox item area and select "Delete toolbox".
You cannot delete the last toolbox (there must always be one).  
If you really must delete the last toolbox, create an empty toolbox before deleting the other one.

To add a vessel to a toolbox, right click in the toolbox area and select "Add vessel" and select
the configuration file of the vessel you want to add.

To delete a vessel, right click in the toolbox area and select "Delete vessel"

How to make the Deltaglider compatible with Stack Editor
--------------------------------------------------------

Stack Editor has only access to config files of vessels. But some vessels, like for example the Deltaglider,
declare their dockports and mehses exclusively in code. Both properties are obviously vital 
for Stack Editor to work with a vessel.

But you cannot just go to the config file and declare the meshname and the DOCKPORT_LIST,
because then the vessel will have a duplicate mesh and dockport when created in Orbiter.
It will not check whether the mesh or the dockport was already loaded.

For this reason, Stack Editor provides alternate names for these two properties. 
You can put them in a vessels config file, and Stack Editor will recognise them, while Orbiter itself will not,
thus avoiding the above described calamities.

These parameter names are:
```
SE_MeshName = meshname
BEGIN_SE_DOCKLIST
END_SE_DOCKLIST
```

The dockport and meshname declarations otherwise work exactly the same as they would for a normal Orbiter parameter.
So let's say you want to use the Deltaglider in Stack Editor, you declare its mesh name and its docking port
in the deltaglider.cfg as follows:

```
SE_MeshName = DG/deltaglider

BEGIN_SE_DOCKLIST
0 -0.49	10.076		0 0 1		0 1 0
END_SE_DOCKLIST
```

Voila, you can now add it to a Toolbox in Stack Editor.
Technically, this would also work with a DG4 or an XR-whatever. The problem with these vessels, however,
is that they have their dockports retracted on spawning, and also have sophisticated damage models:
They will spawn docked, but with closed nosecones, and will pretty much self-destruct as a result of that.
Sorry, but there's no way we can circumvent that.

License
-------
StackEditor is licensed under the MIT license.
See LICENSE for more details.

Reporting bugs
---------------
Please report bugs on the [SE development thread](http://orbiter-forum.com/showthread.php?t=33315)

Including the log, located at ORBITER_ROOT/StackEditor/StackEditor.log would be helpful.
It may also be helpful to change your loglevel to INFO to help with diagnosing the bug.