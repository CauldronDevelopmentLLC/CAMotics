import os
import subprocess

# directory of this script
pwd = os.path.abspath(os.path.expanduser(os.path.dirname(__file__)))
# directory of camotics root
root = os.path.abspath(os.path.join(pwd, ".."))
# location of dockerfile
dockerfile = os.path.join(pwd, "Dockerfile")

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
    "qttools5-dev-tools",
    "scons",
    "sudo",
}
# keyed as docker image : data
images = {"ubuntu:24.04": base}


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

        subprocess.call(command)
