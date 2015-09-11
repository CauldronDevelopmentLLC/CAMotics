This is an example [TPL](http://tplang.org/) program which reads in a DXF file
and cuts along the lines of the first layer.

The ``.svg`` file was converted to ``.dxf`` in Linux with the following
commands:

    inkscape -f camotics.svg -E camotics.eps
    pstoedit -dt -f 'dxf_s:-polyaslines -mm' camotics.eps camotics.dxf

You must have both ``inkscape`` and ``pstoedit`` installed.
