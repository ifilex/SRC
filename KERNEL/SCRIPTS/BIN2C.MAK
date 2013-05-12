!include "..\BASE.MAK"

CFLAGS = -mt -1- -v -vi- -k- -f- -ff- -O -Z -d -I$(INCLUDEPATH) -L$(LIBPATH)

.c.obj:
  $(CC) $(CFLAGS) -c $<

.cpp.obj:
  $(CC) $(CFLAGS) -c $<


EXE_dependencies =  \
 bin2c.obj

production:     bin2c.com

bin2c.com:      $(EXE_dependencies)
                $(LINK) /m/t/c $(LIBPATH)\c0t.obj+bin2c.obj,bin2c,,\
		$(LIBS)+$(CLIB);


clobber:	clean
                $(RM) bin2c.com

clean:
        ..\..\C\rm -f *.obj *.bak *.crf *.xrf *.map *.lst *.las status.me

bin2c.obj: bin2c.c
