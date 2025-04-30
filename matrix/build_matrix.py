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


# keyed as docker image : data
images = {
    "ubuntu:24.04": {
        "build-deps": "scons build-essential libqt5websockets5-dev libqt5opengl5-dev qttools5-dev-tools libnode-dev libglu1-mesa-dev pkgconf git ca-certificates python3-six fakeroot",
        "install-subs": {"libssl1.1": ""},
    }
}


if __name__ == "__main__":
    for image, kwargs in images.items():
        deps = kwargs["build-deps"]

        # build the dockerfile with fully specified paths
        command = [
            "docker",
            "build",
            "--build-arg",
            f"IMAGE={image}",
            "--build-arg",
            f"DEPS={deps}",
            "--output",
            results,
            "-f",
            dockerfile,
            root,
        ]

        print(f"attempting to build: `{image}`")
        print(f"calling with: `{' '.join(command)}`")

        subprocess.call(command)
