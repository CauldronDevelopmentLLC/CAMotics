OpenSCAM is an Open-Source Simulation and Computer Aided Machining software.

See http://openscam.com/

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
