call "c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

cd c:\build\camotics

cd windows-10-64bit-camotics-release
start "camotics-release" slave.bat
cd ..

cd windows-10-64bit-camotics-debug
start "camotics-debug" slave.bat
cd ..
