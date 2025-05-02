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

# base debian packages
base = {
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

# keyed as docker image : data
images = {
    "ubuntu:25.04": base,  # plucky : TODO : cbang can't find v8.h despite installing libnode-dev
    "ubuntu:24.04": base,  # noble
    "ubuntu:22.04": base,  # jammy
    "debian:trixie": base,  # : TODO : same issue as plucky
    "debian:bookworm": base,
    "debian:bullseye": base,
}

# debugging: test with just one image
k = "ubuntu:25.04"
images = {k: images[k]}

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

        # will not raise error if it fails, if you wanted
        # to only accept full results change this to
        # `check_call`
        subprocess.call(command)
