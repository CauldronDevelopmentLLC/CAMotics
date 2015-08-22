import os

class Suite:
    def __init__(self, th):
        cmd = os.path.abspath(th.path + '/../../camoeval')

        th.Test('CoordinateSystem', command = cmd)
        th.Test('Global', command = cmd)
