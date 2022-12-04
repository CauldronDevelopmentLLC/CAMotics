# Setup Linux Buildbot Worker in chroot

## Create and Enter chroot

    sudo debootstrap --arch amd64 stable root
    sudo chroot root

## Install packages

    apt-get update
    apt-get install -y git scons build-essential pkgconf libssl-dev \
      libnode-dev libqt5websockets5-dev libqt5opengl5-dev qttools5-dev-tools \
      libglu1-mesa-dev python3-distutils buildbot-worker

## Install and Start Worker

    mkdir /opt/worker
    cd /opt/worker
    buildbot-worker create-worker . localhost:8012 debian-stable-64bit \
      <password>
    buildbot-worker start

## Build chroot to test AppImage

    sudo debootstrap --arch amd64 stable test

    sudo mkdir test/tmp/.X11-unix test/run/user
    sudo mount --bind /dev test/dev
    sudo mount --bind /proc test/proc
    sudo mount --bind /tmp/.X11-unix test/tmp/.X11-unix
    sudo mount --bind /run/user/ test/run/user

    cp $CAMOTICS_HOME/CAMotics-*.AppImage test/

    sudo chroot root /bin/bash

In the chroot:

    sed -i 's/main/main non-free contrib/' /etc/apt/sources.list
    apt-get update
    apt-get install -y nvidia-driver-libs libfuse2

    ./CAMotics-*.AppImage

    exit

To clean up:

    sudo umount test/dev
    sudo umount test/proc
    sudo umount test/tmp/.X11-unix
    sudo umount test/run/user
