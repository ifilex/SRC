!include "..\BASE.MAK"

MODEL  = s
CFLAGS = -c -m$(MODEL)
AFLAGS = /Mx /Dmem$(MODEL)=1
LIBFLAGS = /c
TERM   =

OBJS   = perif.obj disco.obj vecmx.obj tiempo.obj pcreloj.obj atreloj.obj \
pcrjes.obj atrjes.obj

LIBOBJS= +perif +disco +vecmx +tiempo +pcreloj +atreloj +pcrjes +atrjes

all:            production

production:     drivers.lib


clean:
        $(RM) *.obj *.bak *.crf *.xrf *.map *.lst

drivers.lib : $(OBJS)
        del drivers.lib
        $(LIBUTIL) $(LIBFLAGS) drivers $(LIBOBJS) $(TERM)

.c.obj :
        $(CC) $(CFLAGS) $<

.asm.obj :
        $(NASM) -f obj $<


