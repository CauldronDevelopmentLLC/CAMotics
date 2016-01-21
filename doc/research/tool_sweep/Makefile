DOCS=conical_tool_point_cut.pdf camotics_3_axis_cnc_simulation.pdf
DOCS+=conical_tool_point_cut_math.pdf
FIGS=fig1.eps
EQS=spherical_tool_point_cut.c conical_tool_point_cut.c

EXTS=.toc .aux .log .dvi .eps .eps_tex -eps-converted-to.pdf .svg

F2C=s/\*\*2/2/g;s/\([\*+\/-]\)/ \1 /g;s/  / /g;s/( - /(-/g;
F2C+=s/(\(-[^)]\+\))/\1/g;s/^\(.*\)$$/double \1\;/

all: $(DOCS) $(EQS)

%.pdf: %.tex $(FIGS)
	rm -f $(basename $<).toc
	pdflatex $<
	pdflatex $<

%.eps: %.svg
	inkscape -D -z --file=$< --export-eps=$@ --export-latex

%.f: %.max
	maxima < $<
	mv out.f $@

%.c: %.f
	sed "$(F2C)" $< > $@ || rm $Q

tidy:
	rm -f *~

clean: tidy
	rm -f $(DOCS) $(FIGS) $(EQS) out.f \
	  $(foreach EXT,$(EXTS),$(patsubst %.pdf,%$(EXT),$(DOCS))) \
	  $(foreach EXT,$(EXTS),$(patsubst %.eps,%$(EXT),$(FIGS)))
