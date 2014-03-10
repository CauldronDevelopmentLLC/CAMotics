# 0.2.0:

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

# 0.1.4:

2013-08-12

  - Show correct feed mode constants in feed() error message.

# 0.1.3:

2013-06-30

  - Changed 'spin()' -> 'speed()' in docs.
  - Return current spindle configuration if no args are given to 'speed()'.
  - Return current feed configuration if no args are given to 'feed()'.
  - Return current tool if no args are given to 'tool()'.
  - Fixed relative TPL matrix operations.
  - Added snapshot feature to OpenSCAM GUI.
  - Added icut() and irapid().
  - Fixed play/pause button update.
  - Added loop check box, off by default.
  - Display "inf" when time is infinate due to zero feed.

# 0.1.2:

2013-06-30

  - Initial alpha release of Tool Path Language.

# 0.0.11:

2012-07-14

  - Don't try to lookup references when evaluating assignments.
  - Look up functions in script path.  #57, #44
  - Fixed named subroutine call.  #58
  - Lowercase all o-code names, according to LinuxCNC spec.

# 0.0.10:

2012-03-21

  - Changed to GPU v2+ for LinuxCNC compatibility.
  - Build mostly static binaries.

# 0.0.9:

12012-03-09

  - Don't look for local named refs not starting with '_' in global scope.
  - Specify tool diameter rather than radius in GUI.
  - Added tools shapes ballnose, spheroid & snubnose. #2
  - Implemented ballnose, snubnose and other tool shapes.  #4
  - Allow tools to be specified in mm or inch.  Part of #43.
  - Display diagram of tool in tool editing dialog.

# 0.0.8:

2012-03-03

  - Automatically calculate render resolution based on workpiece.  #3
  - Don't allow unquoted expressions in assignment, not allowed by LinuxCNC.
  - Don't execute an implict motion if a non-modal group 0 code is issued.
  - Fixed global offsets.  G92, G92.1, G92.2, G92.3
  - Added stretching cat array example.
  - Correctly implement named local/global variables in sub routines.
  - Added some tests.

# 0.0.7:

2012-02-20

  - Fixed 3D screen lockup on failed parse of file autoreload.
  - Correct handle local assignment in subroutine.
  - Set tool path position by percent rather than time.  #34
  - Open file dialog in current directory for new projects.  #38, #37
  - Initialize tool front angle, back angle and orientation to zero.
  - Don't set current name in file dialog when loading, Gtk does not like it.
  - Allow opening G-code as new project from GUI.

# 0.0.6:

2012-02-18

  - Case insenstive G-Code.
  - Detect G-Code vs. project files on command line.  #36
  - Compute angle functions in degrees not radians.
  - Fixed operator associativity.
  - Implemented LinuxCNC O-Codes.  #14

# 0.0.5:

2012-02-10

  - Fixed relative NC file path bug when saving new project.

# 0.0.4:

2012-02-09

  - Fixed file dialog modality problem. #27
  - Display ETA during rendering. #25
  - Fixed: Adding an NC file to a new project causes crash.  #26
  - Allow gcode program delimter '%'.
  - Default tool length to 1.
  - Fix: Adding an NC file to a new project gives it an absolute path.  #30
  - Fix: Saving project causes duplicate nc file entries in .xml.  #29
  - Fixed unary gcode expression parsing.
  - Correctly translate/scale G2/G3 moves with oscamtran.
  - Simplify constant g-code expressions in oscamtran.
  - Move render resolution to project and save with project.

# 0.0.3:

2012-01-16

  - Hide as of yet unsupported tool types.
  - Fixed NC file loading.  #26
  - Implemented conical tool.  #5
  - Test if points are inside the cut surface rather than compute distances.
  - Added smoothing which computes normals from adjacent vertices.
  - Fixed slanted conical moves.
  - Improved performance.
  - Added Glut based geometry testbench.

# 0.0.2:

2012-01-11

  - Fixed tool bar icons in Windows.
  - Included correct Gtk .dlls with Windows installer.
  - Added documents to Windows installer.
  - Added disclaimer.

# 0.0.1:

2012-01-11

  - Initial alpha release
