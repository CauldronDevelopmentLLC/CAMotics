call "c:\Program Files\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

cd c:\build\camotics

cd windows-10-32bit-camotics-release
start "camotics-release" slave.bat
cd ..

cd windows-10-32bit-camotics-debug
start "camotics-debug" slave.bat
cd ..
