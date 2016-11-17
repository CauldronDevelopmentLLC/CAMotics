#!/bin/bash -x
# Build a docker image for debian i386.

# settings
arch=i386
suite=${1:-stable}
chroot_dir="/var/chroot/$suite"
apt_mirror="http://http.debian.net/debian"
docker_image="32bit/debian:$suite"

# make sure that the required tools are installed
apt-get update
apt-get install -y docker.io debootstrap dchroot

# install a minbase system with debootstrap
export DEBIAN_FRONTEND=noninteractive
debootstrap --arch $arch $suite $chroot_dir $apt_mirror

# update the list of package sources
cat <<EOF > $chroot_dir/etc/apt/sources.list
deb $apt_mirror $suite main contrib non-free
deb $apt_mirror $suite-updates main contrib non-free
deb http://security.debian.org/ $suite/updates main contrib non-free
EOF

# upgrade packages
chroot $chroot_dir apt-get update
chroot $chroot_dir apt-get upgrade -y

# cleanup
chroot $chroot_dir apt-get autoclean
chroot $chroot_dir apt-get clean
chroot $chroot_dir apt-get autoremove

# create a tar archive from the chroot directory
tar cfz debian.tgz -C $chroot_dir .

# import this tar archive into a docker image:
cat debian.tgz | docker import - $docker_image

# cleanup
rm debian.tgz
rm -rf $chroot_dir
