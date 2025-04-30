import os
import subprocess

cwd = os.path.abspath(os.path.expanduser(os.path.dirname(__file__)))


def distro_string():
    distrib = subprocess.check_output(["lsb_release", "-is"]).decode().lower().strip()
    version = subprocess.check_output(["lsb_release", "-rs"]).decode().lower().strip()

    return f"{distrib}-{version}"


if __name__ == "__main__":
    name = next(n for n in os.listdir(cwd) if n.endswith(".deb"))

    distro = distro_string()

    if distro not in name:
        name_new = f"{name[:-4]}_{distro}.deb"
        os.rename(os.path.join(cwd, name), os.path.join(cwd, name_new))
        print(f"renaming `{name}` -> `{name_new}`")
