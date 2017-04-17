# OpenSSL for Windows

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
    Start-Service sshd

See https://github.com/PowerShell/Win32-OpenSSH/wiki/Install-Win32-OpenSSH

Mount ``c:\`` as follows:

    sshfs -o cache=no,uid=1000 buildbot@10.1.3.30:c:\\ mnt
