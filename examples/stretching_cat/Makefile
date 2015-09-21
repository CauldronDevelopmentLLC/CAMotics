all: stretching_cat.dxf

%.dxf: %.eps
	pstoedit -dt -f 'dxf_s:-polyaslines -mm' $< $@

%.eps: %.svg
	inkscape -f $< -E $@
