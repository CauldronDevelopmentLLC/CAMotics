import os
import subprocess

cwd = os.path.abspath(os.path.expanduser(os.path.dirname(__file__)))


def distro_string():
    """
    Produce a label for the current flavor and version of debian
    that looks like `ubuntu-22.04` or `debian-12.0`.

    There is probably a "debianic" way to do this, but I don't know it.
    And we need the deb files to be named something different so they
    don't overwrite each other when we build them.

    Returns
    -------
    distro
      The distribution name and version, e.g. `ubuntu-22.04`
    """
    # i.e. `ubuntu`
    distrib = subprocess.check_output(["lsb_release", "-is"]).decode().lower().strip()
    # i.e. `22.04`
    # if you wanted this to be like `noble` or `jessie` switch the `-rs` to `-cs`
    version = subprocess.check_output(["lsb_release", "-rs"]).decode().lower().strip()
    # i.e. `trixie`, `plucky`, etc
    code = subprocess.check_output(["lsb_release", "-cs"]).decode().lower().strip()

    # the version may be `n/a`
    result = "-".join(
        c for c in [distrib, version, code] if len(c) > 0 and "/" not in c
    )

    return result


if __name__ == "__main__":
    # get the first deb in this directory
    name = next(n for n in os.listdir(cwd) if n.endswith(".deb"))

    distro = distro_string()

    if distro not in name:
        name_new = f"{name[:-4]}_{distro}.deb"
        os.rename(os.path.join(cwd, name), os.path.join(cwd, name_new))
        print(f"renaming `{name}` -> `{name_new}`")
