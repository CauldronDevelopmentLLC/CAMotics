import os

class Suite:
    def __init__(self, th):
        cmd = os.path.abspath(th.path + '/../../oscameval')

        th.Test('Numeric', command = cmd)
        th.Test('Named', command = cmd)
        th.Test('LocalNumeric', command = cmd)
        th.Test('GlobalNumeric', command = cmd)
        th.Test('LocalNamed', command = cmd)
        th.Test('GlobalNamed', command = cmd)
