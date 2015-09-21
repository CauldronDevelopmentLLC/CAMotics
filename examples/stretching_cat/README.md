This is an example [TPL](http://tplang.org/) program which reads in a DXF file
and cuts along the lines of the first layer.

The ``.svg`` file was converted to ``.dxf`` in Linux with the following
commands:

    inkscape -f stretching_cat.svg -E stretching_cat.eps
    pstoedit -dt -f 'dxf_s:-polyaslines -mm' stretching_cat.eps stretching_cat.dxf

You must have both ``inkscape`` and ``pstoedit`` installed.
