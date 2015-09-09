import os, sys

# local cbang
if not os.environ.get('CBANG_HOME'): os.environ['CBANG_HOME'] = './cbang'
cbang = os.environ.get('CBANG_HOME')

# Version
version = '1.0.0'
major, minor, revision = version.split('.')

# Setup
env = Environment(ENV = os.environ,
                  TARGET_ARCH = os.environ.get('TARGET_ARCH', 'x86'))
env.Tool('config', toolpath = [cbang])
env.CBAddVariables(
    ('install_prefix', 'Installation directory prefix', '/usr/local/'),
    EnumVariable('qt_version', 'Version of Qt to use', 'auto',
                 allowed_values = ('auto', '4', '5')))
env.CBLoadTools(
    'compiler cbang dist freetype2 opengl build_info packager')
conf = env.CBConfigure()

# Config vars
env.Replace(PACKAGE_VERSION = version)
env.Replace(BUILD_INFO_NS = 'CAMotics::BuildInfo')

# Qt5 tool
qt_version = env.get('qt_version')
if qt_version in ('auto', '5'):
    try:
        env.Tool('qt5', toolpath = ['./config'])
        qt_version = '5'
        env.EnableQtModules = env.EnableQt5Modules
        env.Uic = env.Uic5
        env.Qrc = env.Qrc5
    except:
        qt_version = '4'

if qt_version == '4':
    # Qt4 tool
    env.Tool('qt4', toolpath = ['./config'])
    qt_version = '4'
    env.EnableQtModules = env.EnableQt4Modules
    env.Uic = env.Uic4
    env.Qrc = env.Qrc4

# Dist
if 'dist' in COMMAND_LINE_TARGETS:
    env.__setitem__('dist_build', '')

    # Only files check in to Subversion
    lines = os.popen('svn status -v').readlines()
    lines += os.popen('svn status -v cbang').readlines()
    lines = filter(lambda l: len(l) and l[0] in 'MA ', lines)
    files = map(lambda l: l.split()[-1], lines)
    files = filter(lambda f: not os.path.isdir(f), files)

    tar = env.TarBZ2Dist('camotics', files)
    Alias('dist', tar)
    Return()


if not env.GetOption('clean'):
    # Configure compiler
    conf.CBConfig('compiler')

    conf.CBConfig('cbang')
    env.CBDefine('USING_CBANG') # Using CBANG macro namespace

    env.CBDefine('GLEW_STATIC')

    # Qt
    env.EnableQtModules('QtCore QtGui QtOpenGL'.split())
    env.Append(CCFLAGS = ['-fPIC'])

    conf.CBConfig('freetype2')
    conf.CBConfig('opengl')
    conf.CBConfig('v8', True)

    # Cairo
    conf.CBCheckHome('cairo')
    conf.CBRequireLib('cairo')
    if env['PLATFORM'] == 'darwin':
        conf.RequireOSXFramework('CoreText')

    # Include path
    env.AppendUnique(CPPPATH = ['#/src'])

    # Extra static libs
    if env.get('static') or env.get('mostly_static'):
        conf.CBCheckLib('selinux')
        env.ParseConfig('pkg-config --libs --static cairo')

conf.Finish()

# Source
src = ['src/glew/glew.c']
for subdir in [
    '', 'gcode/ast', 'sim', 'gcode', 'probe', 'view', 'opt', 'pcb', 'stl',
    'contour', 'qt', 'cutsim', 'remote', 'render', 'value', 'dxf',
    'dxf/dxflib']:
    src += Glob('src/camotics/%s/*.cpp' % subdir)

for subdir in ['']:
    src += Glob('src/tplang/%s/*.cpp' % subdir)

# Build in 'build'
import re
VariantDir('build', 'src')
src = map(lambda path: re.sub(r'^src/', 'build/', str(path)), src)
env.AppendUnique(CPPPATH = ['#/build'])


# Qt
uic = [env.Uic('build/ui_camotics.h', 'qt/camotics.ui')]
for dialog in 'export about donate new'.split():
    uic.append(env.Uic('build/ui_%s_dialog.h' % dialog,
                       'qt/%s_dialog.ui' % dialog))

qrc = env.Qrc('build/qrc_camotics.cpp', 'qt/camotics.qrc')
src += qrc

# Build Info
info = env.BuildInfo('build/build_info.cpp', [])
AlwaysBuild(info)
src += info


# Build
lib = env.Library('libCAMotics', src)
libs = [lib]
Depends(lib, uic)


# 3rd-party libs
libs.append(env.Library('clipper', Glob('build/clipper/*.cpp')))


docs = ('README.md', 'LICENSE', 'COPYING', 'CHANGELOG.md')
progs = 'camotics gcodetool camprobe camopt tplang camsim'
execs = []
for prog in progs.split():
    p = env.Program(prog, ['build/%s.cpp' % prog] + libs + [qrc])
    env.Install(env.get('install_prefix') + '/bin/', p)
    Default(p)
    execs.append(p)


# Clean
Clean(execs, ['build', 'config.log', 'dist.txt', 'package.txt'])


# Install
env.Alias('install', [env.get('install_prefix')])

description = '''CAMotics is an  Open-Source software which can simulate
3-axis NC machining. It is a fast, flexible and user friendly simulation
software for the DIY and Open-Source community.

At home manufacturing is one of the next big technology revolutions. Much like
the PC was 30 years ago. There have been major advances in desktop 3D printing
(e.g.  Maker Bot) yet uptake of desktop CNCs has lagged despite the
availability of cheap CNC machines. One of the major reasons for this is a
lack of Open-Source simulation and CAM (model to tool path conversion)
software. CAM and NC machine simulation present some very difficult
programming problems as evidenced by 30+ years of academic papers on these
topics. Whereas 3D printing simulation and tool path generation is much
easier. However, such software is essential to using a CNC.

Being able to simulate is a critical part of creating usable CNC tool paths.
Programming a CNC with out a simulator is cutting with out measuring; it's
both dangerous and expensive. With CAMotics you can preview the results of
your cutting operations before you fire up your machine. This will
save you time and money and open up a world of creative possibilities by
allowing you to rapidly visualize and improve upon designs with out wasting
material or breaking tools.'''

# Package checked in examples
examples = []
if 'package' in COMMAND_LINE_TARGETS:
    import subprocess

    cmd = 'git ls-files  examples/'
    p = subprocess.Popen(cmd, shell = True, stdout = subprocess.PIPE)
    examples = p.communicate()[0]
    examples = map(lambda x: [x, x], examples.split())


# Get MSVC redist
msvc_redist = 'vcredist_x86.exe'
msvc_redist_url = 'http://download.microsoft.com/download/d/d/9/' + \
    'dd9a82d0-52ef-40db-8dab-795376989c03/' + msvc_redist
if 'package' in COMMAND_LINE_TARGETS and env['PLATFORM'] == 'win32' and \
        not os.path.exists('build/' + msvc_redist):
    redist = env.CBDownload('build/' + msvc_redist, msvc_redist_url)


# Package
if 'package' in COMMAND_LINE_TARGETS:
    if qt_version == '5': qt_pkgs = 'libqt5core5a, libqt5gui5, libqt5opengl5'
    else: qt_pkgs = 'libqtcore4, libqtgui4, libqt4-opengl'

    pkg = env.Packager(
        'CAMotics',
        version = version,
        maintainer = 'Joseph Coffland <joseph@cauldrondevelopment.com>',
        vendor = 'Cauldron Development LLC',
        url = 'http://camotics.com/',
        license = 'COPYING',
        bug_url = 'http://camotics.com/',
        summary = 'Open-Source Simulation & Computer Aided Machining',
        description = description,
        prefix = '/usr',
        icons = ('osx/camotics.icns', 'images/camotics.png'),
        platform_independent = ('lib'),

        documents = ['README.md', 'CHANGELOG.md'] + examples,
        programs = map(lambda x: str(x[0]), execs),
        desktop_menu = ['CAMotics.desktop'],
        changelog = 'CHANGELOG.md',

        nsi = 'camotics.nsi',
        msvc_redist = msvc_redist,

        deb_directory = 'debian',
        deb_section = 'miscellaneous',
        deb_depends = 'debconf | debconf-2.0, libc6, libbz2-1.0, zlib1g, ' +\
            'libcairo2, ' + qt_pkgs,
        deb_priority = 'optional',
        deb_replaces = 'openscam',

        rpm_license = 'GPLv2+',
        rpm_group = 'Applications/Engineering',
        rpm_requires = 'expat, bzip2-libs, libcairo2, ' + qt_pkgs,
        rpm_obsoletes = 'openscam',

        app_id = 'org.camotics',
        app_resources = [['osx/Resources', '.'], ['lib', 'lib']],
        app_copyright = 'Copyright 2011-2015, Cauldron Development LLC',
        app_signature = 'camo',
        app_other_info = {
            'CFBundleExecutable': 'camotics', # Overrides 'programs'
            'CFBundleIconFile': 'camotics.icns',
            },
        app_finish_cmd = 'macdeployqt',
        pkg_scripts = 'osx/Scripts',
        pkg_resources = 'osx/Resources',
        pkg_distribution = 'osx/distribution.xml',
        pkg_plist = 'osx/pkg.plist',
        )

    AlwaysBuild(pkg)
    env.Alias('package', pkg)
