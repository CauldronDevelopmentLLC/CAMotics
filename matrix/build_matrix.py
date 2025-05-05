import os
import subprocess

# directory of this script
pwd = os.path.abspath(os.path.expanduser(os.path.dirname(__file__)))
# directory of camotics root
root = os.path.abspath(os.path.join(pwd, ".."))
# location of dockerfile
dockerfile = os.path.join(pwd, "Dockerfile")

# where to save generated `.deb` packages
results = os.path.join(pwd, "debs")
os.makedirs(results, exist_ok=True)


# the packages required to build CAMotics on Debian.
build_deps = {
    "build-essential",
    "ca-certificates",
    "fakeroot",
    "git",
    "libglu1-mesa-dev",
    "libnode-dev",
    "libqt5opengl5-dev",
    "libqt5websockets5-dev",
    "ninja-build",
    "pkgconf",
    "python3",
    "python3-six",
    "python3-setuptools",
    "qttools5-dev-tools",
    "lsb-release",
    "scons",
    "sudo",
}

# keyed as docker image : build deps
images = {
    "ubuntu:25.04": build_deps,  # plucky : TODO : cbang can't find v8.h despite installing libnode-dev
    "ubuntu:24.04": build_deps,  # noble
    "ubuntu:22.04": build_deps,  # jammy
    "debian:trixie": build_deps,  # : TODO : same issue as plucky
    "debian:bookworm": build_deps,
    "debian:bullseye": build_deps,
}

# debugging: test with just one image
# k = "debian:trixie"
# images = {k: images[k]}

if __name__ == "__main__":
    for image, deps in images.items():
        # build the dockerfile with fully specified paths
        command = [
            "docker",
            "build",
            "--progress",
            "plain",
            "--build-arg",
            f"IMAGE={image}",
            "--build-arg",
            f"DEPS={' '.join(deps)}",
            "--output",
            results,
            "-f",
            dockerfile,
            root,
        ]

        print(f"attempting to build: `{image}`")
        print(f"calling with: `{' '.join(command)}`")

        # to accept partial results or not change between:
        # `subprocess.call` <-> `subprocess.check_call`
        subprocess.check_call(command)
