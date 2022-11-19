# CAMotics Windows Worker Setup

## Start with 64-bit Windows 2019 on AWS

## Install OpenSSH for Windows

Download from https://github.com/PowerShell/Win32-OpenSSH/releases/latest/.
Extract to ``c:\Program Files\OpenSSH-Win32``.

In an admin shell:

    cd c:\Program Files\OpenSSH-Win32
    powershell -executionpolicy bypass
    .\install-sshd.ps1
    .\ssh-keygen.exe -A
    Start-Service ssh-agent
    .\ssh-add ssh_host_dsa_key
    .\ssh-add ssh_host_rsa_key
    .\ssh-add ssh_host_ecdsa_key
    .\ssh-add ssh_host_ed25519_key
    netsh advfirewall firewall add rule name="SSH Port" dir=in action=allow protocol=TCP localport=22
    .\install-sshlsa.ps1
    Set-Service sshd -StartupType Automatic
    Set-Service ssh-agent -StartupType Automatic
    .\FixHostFilePermissions.ps1 -Confirm:$false
    .\FixUserFilePermissions.ps1 -Confirm:$false
    Start-Service sshd

See https://github.com/PowerShell/Win32-OpenSSH/wiki/Install-Win32-OpenSSH

## Install MSVC 2022 community edition.
C++ with Windows SDK is needed with 10.0.20348.1 SDK.

## Enable downloads from shell

    powershell -command "& {&'Set-ItemProperty' -Path 'HKLM:\SOFTWARE\Microsoft\Internet Explorer\Main' -Name 'DisableFirstRunCustomize' -Value 2}"

## Install Debugging Tools For Windows
 - ``Control Panel -> Programs -> Programs and Features``
 - Select the ``Windows Software Development Kit``
 - ``Modify -> Change -> Check "Debugging Tools For Windows" -> Change``.

# Install Python 3
 - Download https://www.python.org/downloads/windows/
 - Advanced install, for all users.
 - Go to ``Settings -> Manage App Execution Aliases`` turn off Python aliases.

## Install scons, pywin32 & buildbot with pip:
In a MSVC command prompt run:

    python -m pip install --upgrade pip
    pip install buildbot-worker pywin32 scons

## Get OpenSSL
A static build of the openssl library is needed.
Download from https://www.firedaemon.com/download-firedaemon-openssl-1
Unpack in ``c:\build``.

## Install Git
Download Git from https://gitforwindows.org/

## Install NSIS
Download NSIS 3 from https://nsis.sourceforge.io/Download
Add ``c:\Program Files (x86)\NSIS\`` to ``PATH``.

## Install JOM
 - Download http://download.qt.io/official_releases/jom/jom.zip
 - Extract to ``\build``.
 - Add to ``PATH``.

## Get Google Depot Tools
 - Download https://storage.googleapis.com/chrome-infra/depot_tools.zip
 - Unpack ``depot_tools.zip`` to ``/build``.
 - Add ``/build/depot_tools`` to ``PATH``.

## Build v8
Checkout v8 code:

    cd \build
    fetch v8
    cd v8
    git checkout 10.2.154.13
    gclient sync

Then from master:

    scp -r buildbot/workers/windows-10-64bit/v8-config/out \
      buildbot/workers/windows-10-64bit/v8-config/*.patch \
      buildbot@<windows-worker>:/build/v8/

Back on windows worker:

    patch -p1 < v8-10.2.154.13-heap_base_header_and_offsetof.patch
    gn gen out/x64.release
    ninja -C out/x64.release v8_monolith
    gn gen out/x64.debug
    ninja -C out/x64.debug v8_monolith

## Create buildbot worker

    cd \build\camotics
    buildbot-worker create-worker . <master ip>:<port> <worker> <password>

## Install worker service
In an Admin powershell:

    buildbot_worker_windows_service --user .\buildbot --password <buildbot's passwd> --startup auto install
    New-Item -path Registry::HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\services\BuildBot\Parameters
    Set-ItemProperty -path Registry::HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\services\BuildBot\Parameters -Name directories -Value c:\build\camotics
    Start-Service buildbot
