!include "..\BASE.MAK"
RELEASE = 1.00
LIBPATH = .
INCLUDEPATH = 
#AFLAGS      = /Mx /DSTANDALONE=1 
LIBS        =..\ARCH\DRIVERS.LIB ..\LIB\CS.LIB
CFLAGS = -1- -O -Z -d -I..\include -I. \
    -D__STDC__=0;KERNEL;I86;PROTO;ASMSUPT
INITCFLAGS = $(CFLAGS) -zAINIT -zCINIT_TEXT -zPIGROUP

.c.obj:
	$(CC) $(CFLAGS) -c $<

.cpp.obj:
	$(CC) $(CFLAGS) -c $<

.asm.obj:
	$(NASM) $(NASMFLAGS) -f obj $<



EXE_dependencies =  \
 dat.obj      \
 alink.obj    \
 bloque.obj   \
 paro.obj     \
 esc.obj      \
 system.obj   \
 texto.obj  \
 emuid.obj    \
 emudosf.obj  \
 nombres.obj  \
 disco.obj    \
 entrada.obj  \
 falla.obj    \
 hex.obj      \
 utipo.obj    \
 stmfat.obj   \
 ftipo.obj    \
 lectura.obj  \
 oem.obj      \
 meminth.obj  \
 memint.obj   \
 base.obj     \
 2os.obj      \
 irq.obj      \
 2oskrnl.obj  \
 inicio.obj   \
 memoria.obj  \
 pasos.obj    \
 ekernel.obj  \
 net.obj      \
 pais.obj     \
 unp.obj      \
 opcion.obj   \
 imp.obj      \
 corre.obj    \
 com.obj      \
 init.obj     \
 reloj.obj    \
 zip.obj      \
 tiempo.obj   \
 consola.obj


production:     KERNEL.SYS

KERNEL.SYS:     2oskrnl.exe
                exeflat 2oskrnl.exe kernel.sys 0x60

clobber:        clean
                $(RM) 2oskrnl.exe doslx.sys status.me

clean:
                $(RM) *.obj *.bak *.crf *.xrf *.map *.lst

2oskrnl.EXE: $(EXE_dependencies) $(LIBS)
        del 2oskrnl.lib
        $(LIBUTIL) 2oskrnl +entrada +base +bloque +esc +emudosf +texto
        $(LIBUTIL) 2oskrnl +imp +com +disco +falla +utipo +stmfat
        $(LIBUTIL) 2oskrnl +ftipo +lectura +oem +memint +2os
        $(LIBUTIL) 2oskrnl +inicio +system +memoria +pasos +ekernel +pais
        $(LIBUTIL) 2oskrnl +nombres +opcion +init +reloj +zip
        $(LIBUTIL) 2oskrnl +tiempo +consola +meminth +irq +dat
        $(LIBUTIL) 2oskrnl +alink +hex +unp +corre +paro
        $(LIBUTIL) 2oskrnl +emuid +net
        del 2oskrnl.bak
        $(LINK) /m/c/L$(LIBPATH) 2oskrnl,2oskrnl,2oskrnl,2oskrnl+$(LIBS);
        del 2oskrnl.lib

2oskrnl.obj: 2oskrnl.asm direc.inc

texto.obj: texto.asm 2os.inc

imp.obj: imp.asm 2os.inc

com.obj: com.asm 2os.inc

entrada.obj: entrada.asm direc.inc punto.inc

dat.obj: dat.asm direc.inc

alink.obj: alink.asm direc.inc

hex.obj: hex.asm direc.inc

meminth.obj: meminth.asm direc.inc
                             
base.obj: base.asm direc.inc

irq.obj: irq.asm

unp.obj: unp.asm direc.inc

corre.obj: corre.asm direc.inc punto.inc

emuid.obj: emuid.asm direc.inc

system.obj: system.c 
        $(CC) $(INITCFLAGS) -c system.c

oem.obj: oem.c 
        $(CC) $(INITCFLAGS) -c oem.c

inicio.obj: inicio.c 
        $(CC) $(INITCFLAGS) -c inicio.c

bloque.obj: bloque.c 

paro.obj: paro.c 

esc.obj: esc.c 

emudosf.obj: emudosf.c 

nombres.obj: nombres.c 

disco.obj: disco.c 

falla.obj: falla.c 

utipo.obj: utipo.c 

stmfat.obj: stmfat.c 

ftipo.obj: ftipo.c 

lectura.obj: lectura.c 

memint.obj: memint.c 

2os.obj: 2os.c 

memoria.obj: memoria.c 

pasos.obj: pasos.c 

ekernel.obj: ekernel.c 

net.obj: net.c 

pais.obj: pais.c 

opcion.obj: opcion.c

init.obj: init.c

reloj.obj: reloj.c 

zip.obj: zip.c 

tiempo.obj: tiempo.c 

consola.obj: consola.c 
