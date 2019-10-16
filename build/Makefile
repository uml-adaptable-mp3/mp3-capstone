# Note: This Makefile will only work in a windows build environment
#Generic
TARGET          = MyProject.coff
TARGETPATH      = bin
BUILDPATH       = bin
VCC             = ../tools/build_tools/lcc
VSA             = ../tools/build_tools/vsa
VSLINK          = ../tools/build_tools/voplinkg
VSAR            = ../tools/build_tools/vsar
HOMEPATH        = ../VSOS
PROJPATH        = ${CURDIR}/..
PROJNAME        = MP3_Capstone

DRIVERS = $(PROJPATH)/drivers
DRIVER_SRC_DIRS = $(sort $(dir $(wildcard $(DRIVERS)/**/Makefile/)))
EXE = $(DRIVER_SRC_DIRS:$(DRIVERS)/%/=%.ap3)

all: $(EXE)
	@printf "Done."

%.ap3: $(DRIVERS)/%/
	@printf "\n==> BUILD STARTED FOR %s ...\n\n" $@
	$(MAKE) -C $^

	@printf "Copying $@ to loadable...\n"
	cp $^/bin/$@ loadable/

	@printf "\n==> BUILD FINISHED FOR %s\n\n" $@

.PHONY: all clean

clean:
	for dir in $(DRIVER_SRC_DIRS); do $(MAKE) clean -C $$dir; done
