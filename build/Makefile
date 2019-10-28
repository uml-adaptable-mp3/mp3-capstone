# Note: This Makefile will only work in a bash windows build environment (git bash, cygwin terminal, etc.)
# Generic
PROJNAME        = MP3_Capstone
TARGET          = MyProject.coff
TARGETPATH      = loadable
TARGET_SYS      = ${TARGETPATH}/SYS
PROJPATH        = ${CURDIR}/..
CONFIG_DIR      = ${PROJPATH}/config
DRIVERS         = $(PROJPATH)/drivers

# VSOS Files
HOMEPATH        = ${PROJPATH}/VSOS
VENDOR_SYS      = ${PROJPATH}/VSOS/VSOS_357_RootAndLibrariesSourceCode/SYS_everything
VSOS_SHELL_DIR  = ${PROJPATH}/VSOS/VSOS_357_RootAndLibrariesSourceCode/solutions/shell

SYS_FILES = $(shell cat ${CONFIG_DIR}/system_files.txt)
VSOS_SHELL = ${TARGETPATH}/shell.ap3

# Build Tools
VCC             = ${PROJPATH}/tools/build_tools/lcc
VSA             = ${PROJPATH}/tools/build_tools/vsa
VSLINK          = ${PROJPATH}/tools/build_tools/voplinkg
VSAR            = ${PROJPATH}/tools/build_tools/vsar

# Source Code
DRIVER_SRC_DIRS = $(sort $(dir $(wildcard $(DRIVERS)/**/Makefile/)))
EXE = $(DRIVER_SRC_DIRS:$(DRIVERS)/%/=%.ap3)


.PHONY: all clean

all: $(EXE) ${SYS_FILES} ${VSOS_SHELL} CONFIG
	@printf "Done.\n"

%.ap3: $(DRIVERS)/%/
	@printf "\n==> BUILD STARTED FOR %s ...\n\n" $@
	$(MAKE) -C $^

	@printf "Copying $@ to loadable...\n"
	cp $^/bin/$@ loadable/

	@printf "\n==> BUILD FINISHED FOR %s\n\n" $@

${TARGET_SYS}:
	@mkdir $@

# Copy VSOS drivers to loadable/SYS
${SYS_FILES}: ${TARGET_SYS}
	@cp ${VENDOR_SYS}/$@ ${TARGET_SYS}/$@

CONFIG:
	@cp ${CONFIG_DIR}/config.txt ${TARGETPATH}/config.txt
	@cp ${CONFIG_DIR}/VSOS.INI ${TARGETPATH}/VSOS.INI

${VSOS_SHELL}:
	@cp ${VSOS_SHELL_DIR}/shell.ap3 ${VSOS_SHELL}


clean:
	@for dir in $(DRIVER_SRC_DIRS); do $(MAKE) clean -C $$dir; done
	@rm -rf ${TARGETPATH}/*.ap3 ${TARGETPATH}/*.dl3
	@rm -rf ${TARGETPATH}/*.txt ${TARGETPATH}/*.INI
	@rm -rf ${TARGETPATH}/SYS/*.ap3 ${TARGETPATH}/SYS/*.dl3
	@rm -rf ${TARGETPATH}/SYS/*.ini ${TARGETPATH}/SYS/*.sym
