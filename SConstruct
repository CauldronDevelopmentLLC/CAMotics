import os, sys, json

# local cbang
if not os.environ.get('CBANG_HOME'): os.environ['CBANG_HOME'] = './cbang'
cbang = os.environ.get('CBANG_HOME')

# Version
with open('package.json', 'r') as f: pkg_meta = json.load(f)
version = pkg_meta['version']

# Setup
env = Environment(ENV = os.environ,
                  TARGET_ARCH = os.environ.get('TARGET_ARCH', 'x86'))
Export('env')
env.Tool('config', toolpath = [cbang])
env.CBLoadTools(
    'compiler cbang dist opengl dxflib python build_info packager resources')
env.CBAddVariables(
    ('install_prefix', 'Installation directory prefix', '/usr/local/'),
    BoolVariable('qt_deps', 'Enable Qt package dependencies', True),
    ('python_version', 'Set python version', '3'),
    BoolVariable('with_tpl', 'Enable TPL', True),
    BoolVariable('with_gui', 'Enable graphical user interface', True),
    BoolVariable('wrap_glibc', 'Enable GlibC function wrapping', False),
    EnumVariable('cxxstd', 'Set C++ language standard', 'c++17',
      allowed_values = ('c++14', 'c++17', 'c++20')),
    )
conf = env.CBConfigure()


# Config vars
env.Replace(PACKAGE_VERSION   = version)
env.Replace(PACKAGE_AUTHOR    = '%(name)s <%(email)s>' % pkg_meta['author'])
env.Replace(PACKAGE_COPYRIGHT = '2011-2022, Joseph Coffland')
env.Replace(PACKAGE_HOMEPAGE  = pkg_meta['homepage'])
env.Replace(PACKAGE_ORG       = 'Cauldron Development')
env.Replace(PACKAGE_LICENSE   = 'https://www.gnu.org/licenses/gpl-2.0.txt')
env.Replace(BUILD_INFO_NS     = 'CAMotics::BuildInfo')
env.Replace(RESOURCES_NS      = 'CAMotics')

# Qt5 tool
if env['with_gui']:
    env.Tool('qt5', toolpath = ['./config'])
    env.EnableQtModules = env.EnableQt5Modules
    env.Uic = env.Uic5
    env.Qrc = env.Qrc5
    env.Qm  = env.Qm5

# Dist
if 'dist' in COMMAND_LINE_TARGETS:
    env.__setitem__('dist_build', '')

    # Only files check in to Subversion
    lines = os.popen('svn status -v').readlines()
    lines += os.popen('svn status -v cbang').readlines()
    lines = filter(lambda l: len(l) and l[0] in 'MA ', lines)
    files = map(lambda l: l.split()[-1], lines)
    files = list(filter(lambda f: not os.path.isdir(f), files))

    tar = env.TarBZ2Dist('camotics', files)
    Alias('dist', tar)
    Return()


have_cairo, have_dxflib, have_python = False, False, False
if not env.GetOption('clean'):
    # Configure compiler
    conf.CBConfig('compiler')

    if env['compiler_mode'] == 'gnu':
        env.AppendUnique(CXXFLAGS = ['-Wno-deprecated-declarations'])

    conf.CBConfig('cbang')
    env.CBDefine('USING_CBANG') # Using CBANG macro namespace
    for lib in 'mariadbclient snappy leveldb yaml sqlite3'.split():
        if lib in env['LIBS']: env['LIBS'].remove(lib)

    # Include path
    env.AppendUnique(CPPPATH = ['#/src'])

    # Python
    have_python = conf.CBConfig('python', False)

    if env['with_tpl']:
        if not (env.CBConfigEnabled('chakra') or env.CBConfigEnabled('v8')):
            raise Exception(
                'Chakra or V8 support is required, please rebuild C! You may '
                'need to set CHAKRA_CORE_HOME or V8_HOME.')
    else: env.CBDefine('CAMOTICS_NO_TPL')

    if env['with_gui']:
        # Qt
        qtmods = 'QtCore QtGui QtOpenGL QtNetwork QtWidgets QtWebSockets'
        env.EnableQtModules(qtmods.split())
        env.CBDefine(['QT_NO_OPENGL_ES_2', 'GL_GLEXT_PROTOTYPES',
                      'QT_COMPILING_QSTRING_COMPAT_CPP'])

        # OpenGL
        if env['PLATFORM'] == 'win32' or int(env.get('cross_mingw', 0)):
            conf.CBCheckLib('opengl32')
        else: conf.CBCheckLib('GL')

        # Cairo
        have_cairo = \
            conf.CBCheckCHeader('cairo/cairo.h') and conf.CBCheckLib('cairo')

    # DXFlib
    have_dxflib = conf.CBConfig('dxflib', False)

    conf.CBCheckLib('stdc++')

    # Extra static libs
    if env.get('static') or env.get('mostly_static'):
        conf.CBCheckLib('selinux')

    if env['wrap_glibc']:
        funcs = 'log logf exp expf pow powf memcpy'.split()
        flags = list(map(lambda n: '-Wl,--wrap=' + n, funcs))
        env.AppendUnique(LINKFLAGS = flags)
        env.CBDefine(['CBANG_WRAP_GLIBC'])

conf.Finish()


# Build in 'build'
import re
VariantDir('build', 'src', duplicate = False)
env.AppendUnique(CPPPATH = ['#/build'])


# libGCode
src = []
for subdir in ['', 'ast', 'parse', 'interp', 'machine', 'plan', 'plan/bbctrl']:
    src += Glob('src/gcode/%s/*.cpp' % subdir)
src = list(map(lambda path: re.sub(r'^src/', 'build/', str(path)), src))
lib = env.Library('build/libGCode', src)
libGCode = lib
env.Prepend(LIBS = lib)


# libSTL
src = Glob('src/stl/*.cpp')
src = list(map(lambda path: re.sub(r'^src/', 'build/', str(path)), src))
libSTL = env.Library('build/libSTL', src)
env.Prepend(LIBS = libSTL)


# libDXF
src = Glob('src/dxf/*.cpp')
src = list(map(lambda path: re.sub(r'^src/', 'build/', str(path)), src))
libDXF = env.Library('build/libDXF', src)
env.Prepend(LIBS = libDXF)


# Source
src = []
subdirs = ['', 'sim', 'probe', 'opt', 'project', 'contour', 'render']
for subdir in subdirs: src += Glob('src/camotics/%s/*.cpp' % subdir)
if env['with_tpl']: src += Glob('src/tplang/*.cpp')
if env['wrap_glibc']: src += ['src/glibc.c']

src = list(map(lambda path: re.sub(r'^src/', 'build/', str(path)), src))


# Build Info
info = env.BuildInfo('build/build_info.cpp', [])
AlwaysBuild(info)
src += info


# 3rd-party libs
if env['with_tpl']:
    lib = env.Library('build/clipper', Glob('build/clipper/*.cpp'))
    env.Prepend(LIBS = lib)


# DXFlib
if not have_dxflib:
    libDXFlib = SConscript(
        'src/dxflib/SConscript', variant_dir = 'build/dxflib')
    env.Append(LIBS = libDXFlib)


# Build GUI
execs = []
if env['with_gui']:
    subdirs = ['qt', 'view', 'value', 'machine']
    guiSrc = []
    for subdir in subdirs:
        guiSrc += Glob('src/camotics/%s/*.cpp' % subdir)
    guiSrc = map(lambda path: re.sub(r'^src/', 'build/', str(path)), guiSrc)
    guiSrc = list(guiSrc)

    # Qt
    dialogs = '''
      export about donate find new tool settings new_project cam cam_layer
      connect upload
    '''.split()
    uic = [env.Uic('build/ui_camotics.h', 'qt/camotics.ui')]
    for dialog in dialogs:
        uic.append(env.Uic('build/ui_%s_dialog.h' % dialog,
                           'qt/%s_dialog.ui' % dialog))

    # I18n
    langs = Glob('languages/*.ts')
    regex = r'^.*camotics_(.*)\.ts$'
    langs = map(lambda path: re.sub(regex, r'\1', str(path)), langs)

    qm_files = []
    for lang in langs:
        lang_dst = 'build/i18n/camotics_%s.qm' % lang
        lang_src = 'languages/camotics_%s.ts' % lang
        qm = env.Qm(lang_dst, lang_src)
        qm_files.append(qm)

    def build_i18n_qrc(target, source, env):
        with open(str(target[0]), 'w') as f:
            f.write('<RCC><qresource prefix="/">\n')
            for src in source:
                src = str(src).replace('\\', '/').replace('build/', '')
                f.write('  <file>%s</file>\n' % src)
            f.write('</qresource></RCC>\n')

    env.Append(BUILDERS = {'I18NQRC': env.Builder(action = build_i18n_qrc)})
    i18n_qrc = env.I18NQRC('build/i18n.qrc', qm_files)
    qrc = [env.Qrc('build/qrc_i18n.cpp', i18n_qrc)]

    # Qt Resources
    qrc += [env.Qrc('build/qrc_camotics.cpp', 'qt/camotics.qrc')]

    # Resources
    res = env.Resources('build/resources.cpp', ['#/src/resources'])
    Precious(res)
    guiSrc.append(res)

    # GUI lib
    guiLib = env.Library('build/libCAMoticsGUI', src + guiSrc)
    env.Prepend(LIBS = [guiLib])
    Depends(guiLib, uic)

    _env = env.Clone()

    # Cairo
    if not have_cairo:
        lib = SConscript('src/cairo/SConscript', variant_dir = 'build/cairo')
        _env.Append(LIBS = [lib])

    if int(_env.get('cross_mingw', 0)):
        _env.AppendUnique(LINKFLAGS = ['-Wl,--subsystem,windows'])

    # GUI progs
    for prog in 'camotics camsim'.split():
        p = _env.Program(prog, ['build/%s.cpp' % prog] + qrc)
        _env.Precious(p)
        _env.Install(_env.get('install_prefix') + '/bin/', p)
        Default(p)
        execs.append(p)

    # Remove GUI libs from remaining programs
    for lib in list(env['LIBS']):
        if str(lib).startswith('Qt'): env['LIBS'].remove(lib)
    for lib in 'cairo GL opengl32'.split():
        if str(lib) in env['LIBS']: env['LIBS'].remove(lib)


# Build lib
libCAMotics = env.Library('build/libCAMotics', src)
env.Prepend(LIBS = libCAMotics)


# Force two pass link to resolve circular dependencies
if env['PLATFORM'] == 'posix':
    env['_LIBFLAGS'] = \
        '-Wl,--start-group ' + env['_LIBFLAGS'] + ' -Wl,--end-group'


# Build other programs
docs = ['README.md', 'LICENSE', 'COPYING', 'CHANGELOG.md']
progs = 'gcodetool planner'
if env['with_tpl']: progs += ' tplang'

for prog in progs.split():
    p = env.Program(prog, ['build/%s.cpp' % prog])
    env.Precious(p)
    env.Install(env.get('install_prefix') + '/bin/', p)
    Default(p)
    execs.append(p)


# Python module
misc_files = []
if have_python:
    # Python source
    src = Glob('src/python/*.cpp')
    src = list(map(lambda path: re.sub(r'^src/', 'build/', str(path)), src))
    lib = env.Library('build/libCAMoticsPy', src)

    env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1

    env.Prepend(LIBS = [lib])
    mod = env.SharedLibrary('build/camotics', ['build/python.cpp'],
                            SHLIBPREFIX = '')
    Default(mod)
    misc_files.append([str(mod[0]), 'usr/lib/python3/dist-packages/'])

# Clean
Clean(execs, ['build', 'config.log', 'dist.txt', 'package.txt'])

# Install
prefix = env.get('install_prefix')
env.Alias('install', [prefix])

for target in docs + ['examples']:
    env.Install(prefix + '/share/doc/camotics/', target)


env.Install(prefix + '/share/camotics/', 'machines')
env.Install(prefix + '/share/camotics/', 'tpl_lib')
env.Install(prefix + '/share/pixmaps', 'images/camotics.png')
env.Install(prefix + '/share/applications', 'CAMotics.desktop')
env.InstallAs(prefix + '/share/mime/packages/camotics.xml', 'mime.xml')
env.Install(prefix + '/share/appdata/', 'CAMotics.appdata.xml')
env.Install(prefix + '/share/icons/hicolor/128x128/apps/', 'images/camotics.png')

description = '''CAMotics is an Open-Source software which can simulate
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

# Package
if 'package' in COMMAND_LINE_TARGETS:
    import subprocess

    # Examples
    examples = []
    cmd = 'git ls-files examples/'
    p = subprocess.Popen(cmd, shell = True, stdout = subprocess.PIPE)
    examples = p.communicate()[0]
    if isinstance(examples, bytes): examples = examples.decode()
    examples = list(map(lambda x: [x, x], examples.split()))

    # Machines
    machines = []
    cmd = 'git ls-files machines/'
    p = subprocess.Popen(cmd, shell = True, stdout = subprocess.PIPE)
    machines = p.communicate()[0]
    if isinstance(machines, bytes): machines = machines.decode()
    machines = list(map(lambda x: [x, x], machines.split()))

    # Code sign key password
    path = os.environ.get('CODE_SIGN_KEY_PASS_FILE')
    if path is not None:
        code_sign_key_pass = open(path, 'r').read().strip()
    else: code_sign_key_pass = None

    if 'SIGNTOOL' in os.environ: env['SIGNTOOL'] = os.environ['SIGNTOOL']

    # Qt dependencies
    if 'QTDIR' in os.environ: env['QTDIR'] = os.environ['QTDIR']

    install_files = []
    if env.get('qt_deps'):
        qt_pkgs = ', libqt5core5a, libqt5gui5, libqt5opengl5, libqt5websockets5'

        if env['PLATFORM'] == 'win32':
            import shutil
            try:
                shutil.rmtree('build/win32')
            except: pass

            cmd = [env['QTDIR'] + '\\bin\\windeployqt.exe', '--dir',
                   'build\\win32', '--no-system-d3d-compiler', '--no-opengl-sw',
                   '--release', '--no-translations', 'camotics.exe']

            if subprocess.call(cmd):
                raise Exception('Call to windeployqt failed')

            for name in os.listdir('build/win32'):
                if 'vcredist' in name or 'vc_redist' in name:
                    env['VCREDIST'] = name
                install_files.append('build\\win32\\' + name)

    else: qt_pkgs = ''

    pkg = env.Packager(
        'CAMotics',
        version = version,
        maintainer = env['PACKAGE_AUTHOR'],
        vendor = env['PACKAGE_ORG'],
        url = env['PACKAGE_HOMEPAGE'],
        license = 'COPYING',
        bug_url = 'https://github.com/CauldronDevelopmentLLC/CAMotics/issues/',
        summary = 'Open-Source Simulation & Computer Aided Machining',
        description = description,
        prefix = '/usr',
        icons = ('osx/camotics.icns', 'images/camotics.png'),
        mime = [['mime.xml', 'camotics.xml']],
        platform_independent = ['tpl_lib'],
        misc = misc_files,

        documents = ['README.md', 'CHANGELOG.md'] + examples + machines,
        programs = list(map(lambda x: str(x[0]), execs)),
        desktop_menu = ['CAMotics.desktop'],
        changelog = 'CHANGELOG.md',

        nsi = 'camotics.nsi',
        nsis_install_files = install_files,
        timestamp_url = 'http://timestamp.comodoca.com/authenticode',
        code_sign_key = os.environ.get('CODE_SIGN_KEY', None),
        code_sign_key_pass = code_sign_key_pass,

        deb_directory = 'debian',
        deb_section = 'miscellaneous',
        deb_depends =
        'debconf | debconf-2.0, libc6, libglu1, ' +
        'libv8-3.14.5 | libv8-dev | libnode-dev, ' +
        'libglu1-mesa, libssl1.1' + qt_pkgs,
        deb_priority = 'optional',
        deb_replaces = 'openscam',

        rpm_license = 'GPLv2+',
        rpm_group = 'Applications/Engineering',
        rpm_requires = 'expat' + qt_pkgs,
        rpm_obsoletes = 'openscam',

        app_id = 'org.camotics.CAMotics',
        app_resources = [['osx/Resources', '.'], ['tpl_lib', 'tpl_lib'],
                        ['osx/entitlements.plist', '.']],
        app_copyright = env['PACKAGE_COPYRIGHT'],
        app_signature = 'camo',
        app_other_info = {
            'CFBundleExecutable': 'camotics', # Overrides 'programs'
            'CFBundleIconFile': 'camotics.icns',
            'CFBundleDocumentTypes': [{
                'CFBundleTypeExtensions': ['camotics'],
                'CFBundleTypeRole': 'Editor',
                }],
            },
        app_finish_cmd = 'osx/deployqt',
        pkg_scripts = 'osx/Scripts',
        pkg_resources = [['osx/Resources', '.'],
                      ['images/camotics-pkg-bg.png', 'en.lproj/camotics.png']],
        pkg_distribution = 'osx/distribution.xml',
        pkg_plist = 'osx/pkg.plist',
        sign_apps = ['Applications/CAMotics.app'],
        )

    AlwaysBuild(pkg)
    env.Alias('package', pkg)
