# CAMotics Changelog

## v1.3.0:
 - Multi-language support.
 - German language translation. (Joël Plüss)
 - Look for O-Code program files with ``.ngc`` extension as well as with none.
 - Debian package improvements and more. (Sergey Mende)
 - Mime file install fix + C++17 support. (Paul Hentschel)
 - Improved automatic workpiece generation.
 - Typo fixes (@tpimh & @lzpaz)

## v1.2.1:
 - Implemented tool offsets (G43, G43.1, G43.2, G49)
 - Implemented coord XY rotation (G10 L2)
 - Added warning for G70 & G71
 - Fixes for TPL arc()
 - Always output a decimal point for compatibility with Haas and Fanuc. #298
 - Respect offsets in canned cycle moves.
 - Emit error on any tool operation with no tool selected.
 - Emit error but set default when feed rate not set and cutting move is made.
 - Allow dot in GCode variable reference names.
 - Warn about rendering empty workpiece.
 - Move all axes on "G28/G30 <axes>" as specified in RS274/NGC section 3.5.8.
 - Segments straddle arc in linearization.
 - Control max-arc-error with GCode var.
 - Implemented path modes G61, G61.1 & G64 with naive CAM but not blending, yet.
 - Handle LASER intensity correctly for M4 as well as M3.
 - Implemented ``(DEBUG,)``, ``(PRINT,)`` & ``(LOG,)`` comments.
 - Better handling of spaces in GCode named reference.
 - Fixed default tool table loading.
 - Don't output synchronization warning after a program pause.
 - Resolve simlinks in file dialog.
 - Allow running planner in CAMotics GUI.
 - Fixed swapped G98 and G99.

## v1.2.0:
 - Improved Buildbotics CNC controller connectivity.
 - Revamped donation dialog.
 - Show donation dialog once at start with each upgrade.
 - S-Curve path planner.
 - Ignore Program Number O-Codes used by some post processors.
 - Added intensity view for LASER cuts.
 - Added seek and homing G-Codes.
 - Tabbing fixes. (hpmachining)
 - Toggle console. (hpmachining)
 - New tool menu option. (hpmachining)
 - Zoom menu option. (hpmachining)
 - Keyboard shortcuts. (hpmachining)
 - More logical view button order. (hpmachining)
 - Show feed in project units. (hpmachining)
 - Handle long or even infinite GCode programs better.
 - Implemented M70-M73
 - More direct perspective view.
 - Fixed view reset during rotation bug.
 - Added Ctrl-R for reload/run.
 - Fixed recent projects list.
 - Set default new project name from first file added.
 - Several fixes for tool editor.
 - Several project file handling fixes.
 - Allow outputting GCode with CRLF Windows line endings. #286
 - Improved about, welcome and help dialogs.
 - Improved status messages and progress bar.
 - Automatically scale GUI for high-res monitors.
 - Require mesa OpenGL in Debian package.
 - Fixed problem with real-time simulation. #264
 - Added LASER example La Peinture.

## v1.1.2:
 - Fixed ``Could not save project: Invalid file tab index 0``. #225
 - Fixed tool dialog editing problems.  #79
 - Moved ``Length`` after ``Diameter`` in tool dialog.
 - Fix bounding box calculation for spheroid tools.
 - Stop simulation if file save is canceled.
 - Added more examples
 - Fixed error in calculating cuts with bottom of cylinder / snub.
 - Added support for tabs in 2D profile cuts in dfx TPL lib.
 - Added Buildbotics controller connectivity.
 - Fixed modal dialogs.
 - Fixed Windows OpenGL problems.
 - Dropped support for Qt4.
 - Change from XML to JSON project file format and .camotics extension.
 - Show 3D view of machine.

## v1.1.1:
 - Fixed bug preventing ``camsim`` from writing STL data.

## v1.1.0:
 - Ask about units and tool table when creating new projects.
 - Real-time simulation. #2
 - Handle UTF8 file names correctly. #190
 - Handle file names with spaces correctly. #167
 - Support .tap file extension. #189
 - Generate M6 when tool 1 is first used. #188
 - Fixed jagged arcs caused by foreign locale setting. #180
 - Fixed simulation playback speed. #179
 - Fixed conical tool (v-bit) simulation. #173
 - Allow entering conical tool in degrees + length. #65
 - Return all tool details in with TPL tool() function. #127
 - Surface is made translucent when moving position slider.
 - Disabled signal handler on command line tools for better CTRL-C handling.

## v1.0.6:
 - 4.5x simulation speed up.
 - Fix for missing tool paths in Windows 8.1 with Intel HD4000.

## v1.0.5:
 - Add option to disable VBOs to help with faulty graphics drivers.

## v1.0.4:
 - Clear surface on reload.
 - Cutting move with zero feed is now an error.
 - No error on rapid move with zero feed.
 - Allow tool zero.
 - Import/export tool table.
 - Improved tool table editing.
 - Attempt to fix missing tool paths in Windows 8.1 with Intel HD4000.
 - Default tool table.
 - Simplified user interface.
 - Moved project units and resolution to settings dialog.
 - Default units, metric or imperial.
 - Output line numbers and files in GCode.
 - Fixed tool path play back problems.

## v1.0.3:
 - Handle non-latin characters in filenames.
 - Fixed stretching cat example.
 - Fixed G90.1 mode in Windows.

## v1.0.2:
 - Implemented absolute arc mode
 - Implemented incremental distance mode
 - Fix predefined position moves (G28 & G30)
 - Handle UTF-8 Byte Order Marker
 - Allow exporting GCode without simulated surface
 - Don't warn about zero feed on rapid moves

## v1.0.1:
 - Find & replace in editor
 - Find in console
 - Export correct GCode from GUI
 - Fixed TPL module.exports handling

## v1.0.0:
 - Added GCode/TPL editor/viewer #89
 - Added console in GUI
 - Warn and popup console on errors
 - Double click on error in console jumps to line in code
 - Fixed problems with editing workpiece values
 - All jobs now run in the background
 - Provide progress feedback when large GCode files are being loaded
 - Auto load a demo project #104
 - Output 'M2' instead of '%' at end of GCode
 - Ability to reduce the number of triangles in the simulated workpiece
 - Allow exporting GCode from GUI
 - Ability to run simulations from the command line
 - Fixed STL export in Windows #81
 - Fixed problems running with different locale settings #100
 - Fixed v8 linking problems
 - Fixed problems with automatic workpiece z-axis #105
 - Fixed filename problems when opening and saving files #87
 - Fixed Ubuntu 12.04 crashes #86
 - Preliminary support for TPL and TPL libraries

## v0.2.5:
 - Fixed arcs.  #78, #85

## v0.2.4:
 - Removed dependency on libbfd.  #84

## v0.2.3:
 - Fixed valgrind uninitialized var warning.
 - Added missing DLLs to Windows installer.

## v0.2.2:
 - Remove old cut surface after project change.
 - Ask user to open .xml project file if it exists.
 - Added link to online help.

## v0.2.1:

2014-03-22

 - Generate tool paths in background.
 - Added icon to indicate run status.
 - Moved simulation control buttons to bottom of sim view.

## v0.2.0:

2014-03-10

 - Support for OSX.
 - New Qt GUI, with configurable layout.
 - Single combined GUI window.
 - Auto update resolution and automatic workpiece.
 - Project wide units (mm/in) setting.
 - Improved workpiece view and editing.
 - Improved tool editing.
 - Easy access to examples in File menu.
 - Added workpiece and tool path bounds display.
 - Added Revert option to File menu.
 - Fixed program line display.
 - Reduced dependencies for easier builds from source.
 - Don't show partial simulations.
 - Clear surface when loading new project.
 - Added many new examples.
 - Added in app donation info.
 - Handle missing tools more gracefully.
 - Fixed ballnose cutter.  #70

## v0.1.4:

2013-08-12

 - Show correct feed mode constants in feed() error message.

## v0.1.3:

2013-06-30

 - Changed 'spin()' -> 'speed()' in docs.
 - Return current spindle configuration if no args are given to 'speed()'.
 - Return current feed configuration if no args are given to 'feed()'.
 - Return current tool if no args are given to 'tool()'.
 - Fixed relative TPL matrix operations.
 - Added snapshot feature to CAMotics GUI.
 - Added icut() and irapid().
 - Fixed play/pause button update.
 - Added loop check box, off by default.
 - Display "inf" when time is infinite due to zero feed.

## v0.1.2:

2013-06-30

 - Initial alpha release of Tool Path Language.

## v0.0.11:

2012-07-14

 - Don't try to lookup references when evaluating assignments.
 - Look up functions in script path.  #57, #44
 - Fixed named subroutine call.  #58
 - Lowercase all o-code names, according to LinuxCNC spec.

## v0.0.10:

2012-03-21

 - Changed to GPU v2+ for LinuxCNC compatibility.
 - Build mostly static binaries.

## v0.0.9:

12012-03-09

 - Don't look for local named refs not starting with '_' in global scope.
 - Specify tool diameter rather than radius in GUI.
 - Added tools shapes ballnose, spheroid & snubnose. #2
 - Implemented ballnose, snubnose and other tool shapes.  #4
 - Allow tools to be specified in mm or inch.  Part of #43.
 - Display diagram of tool in tool editing dialog.

## v0.0.8:

2012-03-03

 - Automatically calculate render resolution based on workpiece.  #3
 - Don't allow unquoted expressions in assignment, not allowed by LinuxCNC.
 - Don't execute an implicit motion if a non-modal group 0 code is issued.
 - Fixed global offsets.  G92, G92.1, G92.2, G92.3
 - Added stretching cat array example.
 - Correctly implement named local/global variables in sub routines.
 - Added some tests.

## v0.0.7:

2012-02-20

 - Fixed 3D screen lockup on failed parse of file autoreload.
 - Correct handle local assignment in subroutine.
 - Set tool path position by percent rather than time.  #34
 - Open file dialog in current directory for new projects.  #38, #37
 - Initialize tool front angle, back angle and orientation to zero.
 - Don't set current name in file dialog when loading, Gtk does not like it.
 - Allow opening G-code as new project from GUI.

## v0.0.6:

2012-02-18

 - Case insensitive G-Code.
 - Detect G-Code vs. project files on command line.  #36
 - Compute angle functions in degrees not radians.
 - Fixed operator associativity.
 - Implemented LinuxCNC O-Codes.  #14

## v0.0.5:

2012-02-10

 - Fixed relative NC file path bug when saving new project.

## v0.0.4:

2012-02-09

 - Fixed file dialog modality problem. #27
 - Display ETA during rendering. #25
 - Fixed: Adding an NC file to a new project causes crash.  #26
 - Allow gcode program delimter '%'.
 - Default tool length to 1.
 - Fix: Adding an NC file to a new project gives it an absolute path.  #30
 - Fix: Saving project causes duplicate nc file entries in .xml.  #29
 - Fixed unary gcode expression parsing.
 - Correctly translate/scale G2/G3 moves with camotran.
 - Simplify constant g-code expressions in camotran.
 - Move render resolution to project and save with project.

## v0.0.3:

2012-01-16

 - Hide as of yet unsupported tool types.
 - Fixed NC file loading.  #26
 - Implemented conical tool.  #5
 - Test if points are inside the cut surface rather than compute distances.
 - Added smoothing which computes normals from adjacent vertices.
 - Fixed slanted conical moves.
 - Improved performance.
 - Added Glut based geometry testbench.

## v0.0.2:

2012-01-11

 - Fixed tool bar icons in Windows.
 - Included correct Gtk .dlls with Windows installer.
 - Added documents to Windows installer.
 - Added disclaimer.


## v0.0.1:

2012-01-11

 - Initial alpha release
