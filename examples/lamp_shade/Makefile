TARGETS=top.ngc side.ngc

all: $(TARGETS)

%.ngc: %.ngc.in
	ncpp $< > $@

clean:
	rm -f $(TARGETS)
