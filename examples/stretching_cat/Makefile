TARGETS=stretching_cat.ngc

all: $(TARGETS)

%.ngc: %.ngc.in
	ncpp $< > $@

clean:
	rm -f $(TARGETS)
