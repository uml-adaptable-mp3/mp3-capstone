#Generic
TARGET          = MyProject.coff
TARGETPATH      = bin
BUILDPATH       = bin
VCC             = ../.ci/bin/lcc
VSA             = ../.ci/bin/vsa
VSLINK          = ../.ci/bin/voplinkg
VSAR            = ../.ci/bin/vsar
HOMEPATH        = ../.ci
HOMEPATH_WIN    = E:/Program Files (x86)/VSIDE
PROJPATH        = ${CURDIR}
PROJNAME        = MP3_Capstone
IF_DEPLOY       =
OBJS            = $(HOMEPATH)/libvs1005h_vsos3/vsos03.o $(BUILDPATH)/battery.o $(BUILDPATH)/buttons.o $(BUILDPATH)/devByteBus.o $(BUILDPATH)/ID3v2.o $(BUILDPATH)/lcd-ili9341.o $(BUILDPATH)/main.o $(BUILDPATH)/taskandstack.o $(BUILDPATH)/UART.o $(BUILDPATH)/UI.o

#Configuration defaults
INCPATHS        = -I../include -I$(HOMEPATH)/libvs1005h_vsos3
LIBPATHS        = -L. -Llib -L$(HOMEPATH)/libvs1005h_vsos3
DEFINES         = -DDEBUG
VCCFLAGS        = -g -hhw_desc -Wall -O6 -fsmall-code

VSAFLAGS        = -c hw_desc -I $(HOMEPATH)/libvs1005h_vsos3
VSLINKFLAGS     = -k -m mem_desc_app02.mem
LIBS            = -lvsos03 -lc -lgeneral -lcodecmpgsmall -lrtossmall

all: $(TARGETPATH)/$(TARGET)

clean:
	del /F /Q $(BUILDPATH)\*.o
	del /F /Q $(BUILDPATH)\*.a
	del /F /Q loadable.ap3

$(TARGETPATH)/$(TARGET): $(OBJS)
	$(VSLINK) $(VSLINKFLAGS) $(OBJS) -o $(TARGETPATH)/$(TARGET) $(LIBPATHS) $(LIBS)

$(BUILDPATH)/battery.o: $(PROJPATH)/../src/battery.c
	$(VCC) $(VCCFLAGS) $(DEFINES) $(INCPATHS) -o $(BUILDPATH)/battery.o $(PROJPATH)/../src/battery.c

$(BUILDPATH)/buttons.o: $(PROJPATH)/../src/buttons.c
	$(VCC) $(VCCFLAGS) $(DEFINES) $(INCPATHS) -o $(BUILDPATH)/buttons.o $(PROJPATH)/../src/buttons.c

$(BUILDPATH)/devByteBus.o: $(PROJPATH)/../src/devByteBus.c
	$(VCC) $(VCCFLAGS) $(DEFINES) $(INCPATHS) -o $(BUILDPATH)/devByteBus.o $(PROJPATH)/../src/devByteBus.c

$(BUILDPATH)/ID3v2.o: $(PROJPATH)/../src/ID3v2.c
	$(VCC) $(VCCFLAGS) $(DEFINES) $(INCPATHS) -o $(BUILDPATH)/ID3v2.o $(PROJPATH)/../src/ID3v2.c

$(BUILDPATH)/lcd-ili9341.o: $(PROJPATH)/../src/lcd-ili9341.c
	$(VCC) $(VCCFLAGS) $(DEFINES) $(INCPATHS) -o $(BUILDPATH)/lcd-ili9341.o $(PROJPATH)/../src/lcd-ili9341.c

$(BUILDPATH)/main.o: $(PROJPATH)/../src/main.c
	$(VCC) $(VCCFLAGS) $(DEFINES) $(INCPATHS) -o $(BUILDPATH)/main.o $(PROJPATH)/../src/main.c

$(BUILDPATH)/taskandstack.o: $(PROJPATH)/../src/taskandstack.c
	$(VCC) $(VCCFLAGS) $(DEFINES) $(INCPATHS) -o $(BUILDPATH)/taskandstack.o $(PROJPATH)/../src/taskandstack.c

$(BUILDPATH)/UART.o: $(PROJPATH)/../src/UART.c
	$(VCC) $(VCCFLAGS) $(DEFINES) $(INCPATHS) -o $(BUILDPATH)/UART.o $(PROJPATH)/../src/UART.c

$(BUILDPATH)/UI.o: $(PROJPATH)/../src/UI.c
	$(VCC) $(VCCFLAGS) $(DEFINES) $(INCPATHS) -o $(BUILDPATH)/UI.o $(PROJPATH)/../src/UI.c

