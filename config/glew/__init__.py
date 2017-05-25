from SCons.Script import *


def configure(conf):
    conf.CBCheckHome('glew')

    conf.CBRequireCHeader('GL/glew.h')
    conf.CBRequireLib('GLEW')


def generate(env):
    env.CBAddConfigTest('glew', configure)


def exists(env): return 1
