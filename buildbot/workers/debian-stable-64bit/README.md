## Create and Enter chroot

    sudo debootstrap --arch amd64 stable root
    sudo chroot root /bin/bash

## Install packages

    apt-get update
    apt-get install -y wget git scons build-essential binutils-dev fakeroot \
      python3-pip debian-keyring debian-archive-keyring ca-certificates git \
      libssl-dev buildbot-worker python-six libqt5websockets5-dev \
      libqt5opengl5-dev libnode-dev libglu1-mesa-dev pkgconf qttools5-dev-tools

## Install and Start Worker

    mkdir /opt/worker
    cd /opt/worker
    buildbot-worker create-worker . localhost:8012 debian-stable-64bit \
      <password>
    buildbot-worker start
