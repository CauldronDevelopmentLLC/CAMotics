import os

class Suite:
    def __init__(self, th):
        cmd = os.path.abspath(th.path + '/../../oscameval')

        th.Test('CoordinateSystem', command = cmd)
        th.Test('Global', command = cmd)
