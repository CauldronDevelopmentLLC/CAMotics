################################################################################
#                                                                              #
#            CAMotics is an Open-Source simulation and CAM software.           #
#    Copyright (C) 2011-2021 Joseph Coffland <joseph@cauldrondevelopment.com>  #
#                                                                              #
#      This program is free software: you can redistribute it and/or modify    #
#      it under the terms of the GNU General Public License as published by    #
#       the Free Software Foundation, either version 2 of the License, or      #
#                      (at your option) any later version.                     #
#                                                                              #
#        This program is distributed in the hope that it will be useful,       #
#         but WITHOUT ANY WARRANTY; without even the implied warranty of       #
#         MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        #
#                  GNU General Public License for more details.                #
#                                                                              #
#       You should have received a copy of the GNU General Public License      #
#     along with this program.  If not, see <http://www.gnu.org/licenses/>.    #
#                                                                              #
################################################################################

from SCons.Script import *

test_program = """
#include <dxflib/dl_dxf.h>
#include <dxflib/dl_creationadapter.h>

#include <iostream>


int main(int argc, char *argv[]) {
  DL_Dxf dxf;
  DL_CreationAdapter adapter;

  // Test for acceptance of std::istream in API
  dxf.in(std::cin, &adapter);

  return 0;
}
"""

def configure(conf):
    conf.CBCheckHome('dxflib')

    conf.CBRequireCXXHeader('dxflib/dl_dxf.h')
    conf.CBRequireLib('dxflib')

    if not conf.TryCompile(test_program, '.cpp'):
        raise Exception('DXFLib missing required features')

    conf.env.CBDefine('HAVE_DXFLIB')


def generate(env):
    env.CBAddConfigTest('dxflib', configure)


def exists(env): return 1
