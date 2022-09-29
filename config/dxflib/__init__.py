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
