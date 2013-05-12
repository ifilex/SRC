#################################################
#                                               #
#     Allegro makefile, by Shawn Hargreaves     #
#                                               #
#     The 'clean' and 'uninstall' targets       #
#     require rm from the fileutils package     #
#                                               #
#################################################

# replace this definition if you are using PGCC
# PGCC=1

.PHONY: baddjgpp baddjdev badgcc badbnu badmake badtxi badpath badalleg

# check that the DJGPP environment variable is set
ifndef DJDIR
baddjgpp:
	@echo Your DJGPP environment variable is not set correctly! It should
	@echo point to the djgpp.env file: see the djgpp readme.1st for details.
endif

# check that the djdev package is installed
ifeq ($(wildcard $(DJDIR)/bin/djasm.exe),)
baddjdev:
	@echo Missing djgpp package! You need to install djdev201.zip (or whatever the
	@echo latest version is). Download this from wherever you got djgpp, and unzip
	@echo it into the root of your djgpp directory.
endif

# check that the gcc package is installed
ifeq ($(wildcard $(DJDIR)/bin/gcc.exe),)
badgcc:
	@echo Missing djgpp package! You need to install gcc2721b.zip (or whatever the
	@echo latest version is). Download this from wherever you got djgpp, and unzip
	@echo it into the root of your djgpp directory.
endif

# check that the binutils package is installed
ifeq ($(wildcard $(DJDIR)/bin/ld.exe),)
badbnu:
	@echo Missing djgpp package! You need to install bnu27b.zip (or whatever the
	@echo latest version is). Download this from wherever you got djgpp, and unzip
	@echo it into the root of your djgpp directory.
endif

# check that the make package is installed
ifeq ($(wildcard $(DJDIR)/bin/gmake.exe),)
badmake:
	@echo Missing djgpp package! You need to install mak3761b.zip (or whatever the
	@echo latest version is). Download this from wherever you got djgpp, and unzip
	@echo it into the root of your djgpp directory.
endif

# check that the texinfo package is installed
ifeq ($(wildcard $(DJDIR)/bin/makeinfo.exe),)
badtxi:
	@echo Missing djgpp package! You need to install txi390b.zip (or whatever the
	@echo latest version is). Download this from wherever you got djgpp, and unzip
	@echo it into the root of your djgpp directory. If you do not need the Info
	@echo documentation, run make all to ignore this error.
endif

# check that djgpp/bin is pathed
ifeq ($(wildcard $(addsuffix /djasm.exe,$(subst ;, ,$(PATH)))),)
badpath:
	@echo Your PATH is not set correctly! This must include the
	@echo djgpp bin directory: see the djgpp readme.1st for details.
endif

# check that Allegro has a good directory structure
ifeq ($(wildcard src/allegro.c),)
badalleg:
	@echo Bad Allegro installation! You did not preserve the directory structure
	@echo while unzipping it: did you remember to use the -d flag with pkunzip?
endif

# autodetect the presence of an executable compressor
ifneq ($(wildcard $(DJDIR)/bin/upx.exe),)
DJP = $(DJDIR)/bin/upx.exe
else
ifneq ($(wildcard $(DJDIR)/bin/djp.exe),)
DJP = $(DJDIR)/bin/djp.exe -s
endif
endif

# set some useful paths
OBJ = obj/djgpp
DOBJ = obj\djgpp
LIB = lib/djgpp/liballeg.a
LIBDEST = $(DJDIR)/lib/liballeg.a
INCDEST = $(DJDIR)/include/allegro.h
DOCDEST = $(DJDIR)/info/allegro.inf
INTERNAL_H = src/internal.h src/djgpp/interndj.h

# autodetect the LFN settings
ifeq ($(LFN),y)
HTML = html
else
HTML = htm
endif

ifdef WARNMODE
WFLAGS = -Wall -W -Werror -Wno-unused
else
WFLAGS = -Wall -Wno-unused
endif

ifdef DEBUGMODE
# build a debug version
OFLAGS = -g
LFLAGS =

else
ifdef PROFILEMODE
# build with profiling information
ifdef PGCC
OFLAGS = -pg -mpentium -O6 -ffast-math
else
OFLAGS = -pg -m486 -O3 -ffast-math
endif
LFLAGS = -pg

else
# build a normal optimised version
ifdef PGCC
OFLAGS = -mpentium -O6 -ffast-math -fomit-frame-pointer
else
OFLAGS = -m486 -O3 -ffast-math -fomit-frame-pointer
endif

ifdef SYMBOLMODE
# keep debug symbols
LFLAGS =

else
# strip debug symbols
LFLAGS =

endif
endif
endif

CFLAGS = -I. -Isrc -I$(OBJ) $(WFLAGS) $(OFLAGS)
SFLAGS = -I. -Isrc -I$(OBJ) $(WFLAGS)

VPATH = demo docs examples setup src tests tools tools/plugins src/djgpp

PROGRAMS = demo keyconf setup afinfo akaitest digitest mathtest miditest \
	   play playfli test vesainfo colormap dat dat2s exedat grabber \
	   pack pat2dat rgbmap examples

SYSOBJS = adlib.o awedata.o bank.o dma.o dpmi.o emu8k.o emu8kmid.o \
	  essaudio.o gpro.o irq.o irqwrap.o joystd.o keyboard.o \
	  mouse.o mpu.o sb.o sndscape.o snespad.o sw.o timer.o vbeaf.o \
	  vbeafex.o vesa.o ww.o

OBJS = allegro.o blit.o blit8.o blit16.o blit24.o blit32.o bmp.o cblend15.o \
       cblend16.o clip3d.o colblend.o color.o config.o cpu.o datafile.o \
       dataregi.o digmid.o dither.o file.o fli.o flood.o fsel.o gfx.o \
       gfx8.o gfx15.o gfx16.o gfx24.o gfx32.o gfxdrv.o graphics.o gsprite.o \
       gui.o guiproc.o inline.o joystick.o joydrv.o lbm.o math.o math3d.o \
       midi.o misc.o mixer.o modesel.o modex.o pcx.o poly3d.o polygon.o \
       quantize.o readbmp.o scanline.o snddrv.o sound.o spline.o sprite.o \
       sprite8.o sprite15.o sprite16.o sprite24.o sprite32.o stream.o \
       stretch.o text.o tga.o vga.o vtable.o vtable8.o vtable15.o vtable16.o \
       vtable24.o vtable32.o xgfx.o $(SYSOBJS)

LIB_OBJS = $(addprefix $(OBJ)/, $(OBJS))

DOCS = NEWS CHANGES docs/changes.$(HTML) \
       faq.txt docs/faq.$(HTML) \
       help.txt docs/help.$(HTML) \
       AUTHORS THANKS docs/thanks.$(HTML) \
       allegro.txt docs/allegro.$(HTML) \
       docs/allegro.txi docs/allegro.inf \
       docs/allegro.rtf

.PRECIOUS: $(OBJ)/%.o

.PHONY: all msg lib install uninstall docs clean veryclean mmxtest $(PROGRAMS)

all: msg $(LIB) $(PROGRAMS) docs install
	@echo All done.
	@echo To use Allegro, #include allegro.h and link with liballeg.a
	@echo Example command line: gcc foobar.c -o foobar.exe -lalleg
	@echo Run make compress to run DJP or UPX on the executable files
	@echo Enjoy!

msg:
	@echo Compiling Allegro. Please wait...

lib: $(LIB)

install: $(LIBDEST) $(INCDEST) $(DOCDEST)

docs: $(DOCS)

$(LIBDEST): $(LIB)
	copy lib\djgpp\liballeg.a $(subst /,\,$(LIBDEST))

$(INCDEST): allegro.h
	copy allegro.h $(subst /,\,$(INCDEST))

$(DOCDEST): docs/allegro.inf
    ifneq ($(wildcard $(DJDIR)/bin/makeinfo.exe),)
	copy docs\allegro.inf $(subst /,\,$(DOCDEST))
    else
	@echo makeinfo not installed: skipping copy of allegro.inf
    endif

$(OBJ)/%.o: %.c allegro.h
	gcc $(CFLAGS) -o $@ -c $<

$(OBJ)/%.o: %.S asmdefs.inc $(OBJ)/asmdef.inc
	gcc $(SFLAGS) -o $@ -c $<

$(OBJ)/%.o: %.s asmdefs.inc $(OBJ)/asmdef.inc
	gcc -x assembler-with-cpp $(SFLAGS) -o $@ -c $<

*/%.exe: $(OBJ)/%.o $(LIB)
	gcc $(LFLAGS) -o $@ $< $(LIB)

docs/%.inf: docs/%.txi
    ifneq ($(wildcard $(DJDIR)/bin/makeinfo.exe),)
	makeinfo --no-split -o $@ $<
    else
	@echo makeinfo not installed: skipping generation of allegro.inf
    endif

docs/%.txi: docs/%._tx $(OBJ)/makedoc.exe
	$(OBJ)/makedoc.exe -txi $@ $<

docs/%.rtf: docs/%._tx $(OBJ)/makedoc.exe
	$(OBJ)/makedoc.exe -rtf $@ $<

docs/%.$(HTML): docs/%._tx $(OBJ)/makedoc.exe
	$(OBJ)/makedoc.exe -$(HTML) $@ $<

%.txt: docs/%._tx $(OBJ)/makedoc.exe
	$(OBJ)/makedoc.exe -ascii $@ $<

NEWS: docs/changes._tx $(OBJ)/makedoc.exe
	$(OBJ)/makedoc.exe -part -ascii NEWS docs/changes._tx

CHANGES: docs/changes._tx $(OBJ)/makedoc.exe
	$(OBJ)/makedoc.exe -ascii CHANGES docs/changes._tx

AUTHORS: docs/thanks._tx $(OBJ)/makedoc.exe
	$(OBJ)/makedoc.exe -part -ascii AUTHORS docs/thanks._tx

THANKS: docs/thanks._tx $(OBJ)/makedoc.exe
	$(OBJ)/makedoc.exe -part -ascii THANKS docs/thanks._tx

$(OBJ)/makedoc.exe: docs/makedoc.c
	gcc $(CFLAGS) $(LFLAGS) -o $@ docs/makedoc.c

$(OBJ)/asmdef.inc: $(OBJ)/asmdef.exe
	$(OBJ)/asmdef.exe $(OBJ)/asmdef.inc

$(OBJ)/asmdef.exe: src/asmdef.c allegro.h $(INTERNAL_H)
	gcc $(CFLAGS) $(LFLAGS) -o $@ src/asmdef.c

mmxtest:
	@echo // no MMX > $(DOBJ)\mmx.h
	@echo .text > $(DOBJ)\mmxtest.s
	@echo emms >> $(DOBJ)\mmxtest.s
	@gcc -c $(OBJ)/mmxtest.s -o $(OBJ)/mmxtest.o
	@echo #define ALLEGRO_MMX > $(DOBJ)\mmx.h
	@echo Your assembler supports MMX instructions!

$(OBJ)/mmx.h:
	@echo Testing for MMX assembler support...
	-$(MAKE) mmxtest

$(OBJ)/setupdat.s $(OBJ)/setupdat.h: setup/setup.dat tools/dat2s.exe
	tools/dat2s.exe setup/setup.dat -o $(OBJ)/setupdat.s -h $(OBJ)/setupdat.h

$(OBJ)/setupdat.o: $(OBJ)/setupdat.s
	gcc $(SFLAGS) -o $(OBJ)/setupdat.o -c $(OBJ)/setupdat.s

setup/setup.exe: $(OBJ)/setup.o $(OBJ)/setupdat.o $(LIB)
	gcc $(LFLAGS) -o setup/setup.exe $(OBJ)/setup.o $(OBJ)/setupdat.o $(LIB)
    ifndef DEBUGMODE
    ifndef SYMBOLMODE
    ifneq ($(DJP),)
	$(DJP) setup/setup.exe
    endif
    endif
    endif

PLUGIN_SOURCES = $(wildcard tools/plugins/*.c)
PLUGIN_OBJS = $(addprefix $(OBJ)/,$(addsuffix .o,$(basename $(notdir $(PLUGIN_SOURCES)))))
DATEDIT_DEPS = $(OBJ)/datedit.o $(PLUGIN_OBJS)
DATEDIT_LINK = $(OBJ)/datedit.o $(PLUGIN_OBJS)

PLUGIN_SCRIPTS = $(wildcard tools/plugins/*.scr)

ifneq ($(PLUGIN_SCRIPTS),)
DATEDIT_DEPS += $(PLUGIN_SCRIPTS)
DATEDIT_LINK += $(addprefix @,$(PLUGIN_SCRIPTS))
endif

$(OBJ)/plugins.h: $(wildcard tools/plugins/*.inc)
	copy tools\plugins\*.inc $(DOBJ)\plugins.h

tools/dat.exe: $(OBJ)/dat.o $(DATEDIT_DEPS) $(LIB)
	gcc $(LFLAGS) -o tools/dat.exe $(OBJ)/dat.o $(DATEDIT_LINK) $(LIB)

tools/dat2s.exe: $(OBJ)/dat2s.o $(DATEDIT_DEPS) $(LIB)
	gcc $(LFLAGS) -o tools/dat2s.exe $(OBJ)/dat2s.o $(DATEDIT_LINK) $(LIB)

tools/grabber.exe: $(OBJ)/grabber.o $(DATEDIT_DEPS) $(LIB)
	gcc $(LFLAGS) -o tools/grabber.exe $(OBJ)/grabber.o $(DATEDIT_LINK) $(LIB)

tools/pat2dat.exe: $(OBJ)/pat2dat.o $(DATEDIT_DEPS) $(LIB)
	gcc $(LFLAGS) -o tools/pat2dat.exe $(OBJ)/pat2dat.o $(DATEDIT_LINK) $(LIB)

$(LIB): $(LIB_OBJS)
	ar rs $(LIB) $(LIB_OBJS)

compress: $(PROGRAMS)
    ifneq ($(DJP),)
	$(DJP) demo/*.exe examples/*.exe tests/*.exe tools/*.exe setup/keyconf.exe obj/djgpp/*.exe
    else
	@echo No executable compressor found! This target requires either the
	@echo DJP or UPX utilities to be installed in your djgpp bin directory.
    endif

clean:
	-rm -v obj/djgpp/*.* lib/djgpp/*.* docs/*.$(HTML) docs/*.txi docs/*.inf docs/*.rtf

veryclean: clean
	-rm -v allegro.txt AUTHORS CHANGES faq.txt help.txt NEWS THANKS \
	       demo/*.exe examples/*.exe setup/*.exe tests/*.exe tools/*.exe

uninstall:
	-rm $(LIBDEST)
	-rm $(INCDEST)
	-rm $(DOCDEST)
	@echo All gone! (sulk)

demo: demo/demo.exe
keyconf: setup/keyconf.exe
setup: setup/setup.exe
afinfo: tests/afinfo.exe
akaitest: tests/akaitest.exe
digitest: tests/digitest.exe
mathtest: tests/mathtest.exe
miditest: tests/miditest.exe
play: tests/play.exe
playfli: tests/playfli.exe
test: tests/test.exe
vesainfo: tests/vesainfo.exe
colormap: tools/colormap.exe
dat: tools/dat.exe
dat2s: tools/dat2s.exe
exedat: tools/exedat.exe
grabber: tools/grabber.exe
pack: tools/pack.exe
pat2dat: tools/pat2dat.exe
rgbmap: tools/rgbmap.exe

examples: examples/ex1.exe examples/ex2.exe examples/ex3.exe \
	  examples/ex4.exe examples/ex5.exe examples/ex6.exe \
	  examples/ex7.exe examples/ex8.exe examples/ex9.exe \
	  examples/ex10.exe examples/ex11.exe examples/ex12.exe \
	  examples/ex13.exe examples/ex14.exe examples/ex15.exe \
	  examples/ex16.exe examples/ex17.exe examples/ex18.exe \
	  examples/ex19.exe examples/ex20.exe examples/ex21.exe \
	  examples/ex22.exe examples/ex23.exe examples/ex24.exe \
	  examples/ex25.exe examples/ex26.exe examples/ex27.exe \
	  examples/ex28.exe examples/ex29.exe examples/ex30.exe \
	  examples/ex31.exe examples/ex32.exe examples/ex33.exe \
	  examples/ex34.exe examples/ex35.exe examples/ex36.exe \
	  examples/ex37.exe examples/ex38.exe examples/ex39.exe \
	  examples/ex40.exe

$(OBJ)/demo.o: demo.h
$(OBJ)/adlib.o: fm_instr.h
$(OBJ)/irq.o: asmdefs.inc $(OBJ)/asmdef.inc
$(OBJ)/sprite.o $(OBJ)/stretch.o: opcodes.h
$(OBJ)/emu8k.o $(OBJ)/emu8kmid.o: emu8k.h
$(OBJ)/dat.o $(OBJ)/datedit.o $(OBJ)/dat2s.o $(OBJ)/grabber.o $(OBJ)/pat2dat.o $(PLUGIN_OBJS): datedit.h
$(OBJ)/datedit.o: $(OBJ)/plugins.h
$(OBJ)/ex12.o $(OBJ)/ex13.o: example.h
$(OBJ)/ex21.o: running.h
$(OBJ)/blit8.o $(OBJ)/blit16.o $(OBJ)/blit24.o $(OBJ)/blit32.o: blit.inc
$(OBJ)/sprite8.o $(OBJ)/sprite15.o $(OBJ)/sprite16.o $(OBJ)/sprite24.o $(OBJ)/sprite32.o: sprite.inc
$(OBJ)/scanline.o $(OBJ)/poly3d.o: $(OBJ)/mmx.h
$(OBJ)/setup.o: $(OBJ)/setupdat.h
$(OBJ)/datafile.o: datafile.h
$(OBJ)/dataregi.o: datafile.h

INTERNAL_DEPS = adlib.o afinfo.o allegro.o blit.o bmp.o clip3d.o config.o \
		cpu.o datedit.o datafile.o dataregi.o digmid.o dma.o \
		essaudio.o file.o fli.o flood.o gfx.o grabber.o graphics.o \
		gui.o guiproc.o inline.o irq.o joystd.o joystick.o \
		keyboard.o keyconf.o lbm.o midi.o mixer.o modex.o mouse.o \
		mpu.o pat2dat.o pcx.o polygon.o readbmp.o sb.o setup.o \
		sndscape.o sprite.o sound.o spline.o stream.o text.o tga.o \
		timer.o vbeaf.o vbeafex.o vesa.o vga.o vtable8.o vtable15.o \
		vtable16.o vtable24.o vtable32.o

$(addprefix $(OBJ)/, $(INTERNAL_DEPS)): $(INTERNAL_H)

