#!/usr/bin/env python

import os
import sys
import re
import subprocess

exclude = set('''
  advapi32.dll kernel32.dll msvcrt.dll ole32.dll user32.dll ws2_32.dll
  comdlg32.dll gdi32.dll imm32.dll oleaut32.dll shell32.dll winmm.dll
  winspool.drv wldap32.dll ntdll.dll d3d9.dll mpr.dll crypt32.dll dnsapi.dll
  shlwapi.dllversion.dll iphlpapi.dll msimg32.dll setupapi.dll
  opengl32.dll glu32.dll wsock32.dll ws32.dll'''.split())


def find_in_path(filename):
    for path in os.environ["PATH"].split(os.pathsep):

        candidate = os.path.join(path.strip('"'), filename)
        if os.path.isfile(candidate): return candidate

        for name in os.listdir(path):
            if name.lower() == filename.lower():
                return os.path.join(path, name)


def find_dlls(path, exclude = set()):
    cmd = ['objdump', '-p', path]
    p = subprocess.Popen(cmd, stdout = subprocess.PIPE)
    out, err = p.communicate()

    for line in out.splitlines():
        if line.startswith('\tDLL Name: '):
            lib = line[11:].strip().lower()
        else: continue

        if not lib in exclude:
            exclude.add(lib)

            path = find_in_path(lib)
            if path is None:
                raise Exception('Lib "%s" not found' % lib)

            yield path

            for x in find_dlls(path, exclude):
                yield x


if __name__ == "__main__":
    for arg in sys.argv[1:]:
        for dll in find_dlls(arg):
            print dll
