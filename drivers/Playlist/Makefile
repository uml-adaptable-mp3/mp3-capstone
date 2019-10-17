# Note: This Makefile will only work in a windows build environment
# Generic
TARGET          = MyProject.coff
TARGETPATH      = bin
BUILDPATH       = bin
PROJNAME        = Playlist
PROJPATH        = ../..
TOOLS_PATH      = $(PROJPATH)/tools/build_tools
VCC             = $(TOOLS_PATH)/lcc.exe
VSA             = $(TOOLS_PATH)/vsa.exe
VSLINK          = $(TOOLS_PATH)/voplinkg.exe
VSAR            = $(TOOLS_PATH)/vsar.exe
VSOS_DIR        = $(PROJPATH)/VSOS

SRC_DIR         = src
INCLUDE_DIR     = include

# Configuration defaults
INCPATHS        = -I$(INCLUDE_DIR) -I$(VSOS_DIR)/libvs1005h_vsos3
LIBPATHS        = -L. -Llib -L$(VSOS_DIR)/libvs1005h_vsos3
DEFINES         = -DDEBUG
VCCFLAGS        = -Werror -Wall -g -h$(PROJPATH)/build/hw_desc -O6 -fsmall-code

VSAFLAGS        = -c $(PROJPATH)/build/hw_desc -I $(VSOS_DIR)/libvs1005h_vsos3
VSLINKFLAGS     = -v -k -m $(PROJPATH)/build/mem_desc_app02.mem
LIBS            = -lvsos03 -lc -lgeneral -lcodecmpgsmall -lrtossmall

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRC:$(SRC_DIR)/%.c=$(BUILDPATH)/%.o)
VSOS_OBJS = $(VSOS_DIR)/libvs1005h_vsos3/vsos03.o

all: $(TARGETPATH)/$(TARGET) $(OBJS)

$(BUILDPATH)/%.o: $(SRC_DIR)/%.c
	$(VCC) $(VCCFLAGS) $(DEFINES) $(INCPATHS) -o $@ $^

$(TARGETPATH)/$(TARGET): $(OBJS)
	$(VSLINK) $(VSLINKFLAGS) $(VSOS_OBJS) $(OBJS) -o $(TARGETPATH)/$(TARGET) $(LIBPATHS) $(LIBS)
	@mv _link_log.txt $(BUILDPATH)
	@mv _link_output_description.txt $(BUILDPATH)
	@mv _link_s_list.txt $(BUILDPATH)
	@mv loadable.ap3 $(BUILDPATH)/$(PROJNAME).ap3

.PHONY: clean
clean:
	rm -f $(BUILDPATH)/*.o
	rm -f $(BUILDPATH)/*.a
	rm -f $(BUILDPATH)/*.txt
	rm -f $(BUILDPATH)/$(PROJNAME).ap3
