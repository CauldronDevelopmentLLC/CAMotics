![OpenSCAM Logo][1]

OpenSCAM is an ​Open-Source software which can simulate 3-axis NC machining. It is a fast, flexible and user friendly simulation software for the DIY and Open-Source community.

At home manufacturing is one of the next big technology revolutions. Much like the PC was 30 years ago. There have been major advances in desktop 3D printing (e.g. ​Maker Bot) yet uptake of desktop CNCs has lagged despite the availability of ​cheap CNC machines. One of the major reasons for this is a lack of Open-Source simulation and CAM (3D model to tool path conversion) software. CAM and NC machine simulation present some very difficult programming problems, as is evidenced by 30+ years of academic papers on these topics. Whereas, 3D printing simulation and tool path generation are much easier. Such software is essential to using a CNC.

Being able to simulate is a critical part of creating CNC tool paths. Programming a CNC with out a simulator is cutting with out measuring; it's both dangerous and expensive. With OpenSCAM you can preview the results of your cutting operation before you fire up your machine. This will save you time and money and open up a world of creative possibilities by allowing you to rapidly visualize and improve upon designs with out wasting material or breaking tools.

See http://openscam.org/

# License
GNU General Public License version 2.  See the file COPYING.

# Prerequisites
  - C! (provided in 'cbang' sub-directory, see C! README for requirements)
  - OpenGL - Open Graphics Library, accelerated 3D graphics
  - GLEW   - http://glew.sourceforge.net/
  - Qt4    - http://qt-project.org/
  - Cairo2 - http://cairographics.org/
  - SCons  - http://www.scons.org/

On Debian based systems all the prerequisites can be installed with the following command line:

    sudo apt-get install scons subversion build-essential libbz2-dev \
      zlib1g-dev libexpat1-dev libssl-dev libboost-dev libsqlite3-dev \
      libxml2-dev libgl1-mesa-dev qt4-dev-tools libqt4-dev libqt4-opengl-dev \
      libcairo2-dev


# Building
After installing the prerequisites.  Build C! with the following command:

    scons -C cbang

Then build OpenSCAM with the following command:

    scons

# Using It

Run the GUI:

    ./openscam

Try out some of the examples in the File menu.


[1]: https://raw.github.com/jcoffland/OpenSCAM/master/images/openscam-logo.png
