!include "..\BASE.MAK"

#CFLAGS = -mt -1- -v -vi- -k- -f- -ff- -O -Z -d -I$(INCLUDEPATH);..\ \
#	 -L$(LIBPATH) -DI86;PROTO;DEBUG
#CFLAGS = -mt -1- -v -vi- -k- -f- -ff- -O -Z -d -I$(INCLUDEPATH);..\include \
#	 -L$(LIBPATH) -DI86;PROTO
CFLAGS = -I$(INCLUDEPATH);..\include \
	 -L$(LIBPATH) -DI86;PROTO

.c.obj:
  $(CC) $(CFLAGS) -c $<

.cpp.obj:
  $(CC) $(CFLAGS) -c $<


LIBS = ..\arch\drivers.lib

EXE_dependencies =  \
 main.obj \
 $(LIBS)

production:    SYS.COM

fat16.h:        fat16.bin bin2c.com
                bin2c fat16.bin fat16.h Mfat16

fat32.h:        fat32.bin bin2c.com
                bin2c fat32.bin fat32.h Mfat32

SYS.COM:      $(EXE_dependencies)
                $(LINK) /m/t/c $(LIBPATH)\c0t.obj+main.obj,sys,,\
		$(LIBS)+$(CLIB);

clobber:	clean
                $(RM) SYS.COM fat16.h fat32.h

clean:
                $(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.las status.me

main.obj: main.c ..\include\port.h ..\include\memoria.h fat16.h fat32.h
