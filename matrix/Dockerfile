# a debian based image, i.e. Ubuntu
# arg set to scratch so it will fail immediately if the build arg is not set
ARG IMAGE="scratch"
FROM ${IMAGE} AS base

# never ask questions in an apt-get install
ENV DEBIAN_FRONTEND=noninteractive

###########################################
FROM base AS build

# required apt packages which may be slightly different
# on different flavors of debian
ARG DEPS=""

RUN apt-get update && \
    apt-get install -y $DEPS

# use /root for checkouts
WORKDIR /root

############################################
# clone and build cbang
RUN git clone --depth=1 https://github.com/CauldronDevelopmentLLC/cbang.git
RUN cd cbang && \
    git checkout d4dd5f52b0d5ee499ec5683f5930b01fdb24884b
# `v8` is provided by libnode-dev
RUN scons -C cbang v8_compress_pointers=false
# assign the environment variable for cbang
ENV CBANG_HOME=/root/cbang

#################################################
# copy the current checkout of camotics into the working directory
# note that this requires that the docker build context is the root of
# the camotics repo i.e. `docker build -f matrix/Dockerfile .`
COPY . CAMotics

# set our working directory to the camotics checkout
WORKDIR /root/CAMotics

# build the executable for camotics
RUN scons

# package the deb
RUN scons package

#################################
# test the debian in an image without the build dependencies
# to make sure that we listed the install dependencies correctly
FROM base AS test

# copy our built package
COPY --from=build /root/CAMotics/camotics*.deb .

# if the deb is not installing cleanly this debug info is helpful: 
# RUN apt-get update && apt-get install aptitude -y && \
#   apt show camotics && aptitude why-not camotics || true

# this should resolve the missing dependencies if we listed them correctly
# if we listed them incorrectly it will remove camotics
RUN apt-get update && apt install --no-install-recommends -y ./*.deb

# this simple smoke test shouldn't crash
RUN camotics --version

# now that we've passed the smoke test rename the deb
# apply the distro information to the deb, i.e. `ubuntu-20.04`
COPY ./matrix/rename.py .
# TODO : should this be in the `debian/control` info?
RUN apt-get install python3 lsb-release -y && python3 rename.py

#########################
# copy the deb into a scratch image so we can eject it in the build command
FROM scratch

# copy renamed binary from test stage as our output
COPY --from=test camotics*.deb .
