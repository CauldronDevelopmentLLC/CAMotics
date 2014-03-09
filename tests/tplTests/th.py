import os
import glob


class Suite:
    def __init__(self, th):
        cmd = os.path.abspath(th.path + '/../../tplang')

        for test in glob.glob('*Test'):
            th.Test(test, command = cmd)
