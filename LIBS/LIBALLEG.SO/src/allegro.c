/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Various bits and pieces for setting up and cleaning up the system.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#ifdef DJGPP
#include <io.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <dos.h>
#include <crt0.h>
#include <sys/segments.h>
#endif

#include "internal.h"



/* in case you want to report version numbers */
char allegro_id[] = "Allegro " ALLEGRO_VERSION_STR ", by Shawn Hargreaves, " ALLEGRO_DATE_STR;


/* error message for sound and gfx init routines */
char allegro_error[80] = "";


/* flag for how many times we have been initialised */
int _allegro_count = 0;


/* flag for 'win-friendly' mode (note the default value :-) */
int i_love_bill = FALSE;


/* the graphics driver currently in use */
GFX_DRIVER *gfx_driver = NULL;


/* abilities of the current graphics driver */
int gfx_capabilities = 0;


/* a bitmap structure for accessing the physical screen */
BITMAP *screen = NULL;


/* info about the current graphics drawing mode */
int _drawing_mode = DRAW_MODE_SOLID;
BITMAP *_drawing_pattern = NULL;
int _drawing_x_anchor = 0;
int _drawing_y_anchor = 0;
unsigned int _drawing_x_mask = 0;
unsigned int _drawing_y_mask = 0;


PALLETE black_pallete;
PALLETE _current_pallete; 

RGB black_rgb = { 0, 0, 0, 0 };

PALLETE desktop_pallete = {
   { 63, 63, 63, 0 },   { 63, 0,  0,  0 },
   { 0,  63, 0,  0 },   { 63, 63, 0,  0 },
   { 0,  0,  63, 0 },   { 63, 0,  63, 0 },
   { 0,  63, 63, 0 },   { 16, 16, 16, 0 },
   { 31, 31, 31, 0 },   { 63, 31, 31, 0 },
   { 31, 63, 31, 0 },   { 63, 63, 31, 0 },
   { 31, 31, 63, 0 },   { 63, 31, 63, 0 },
   { 31, 63, 63, 0 },   { 0,  0,  0,  0 }
};


/* a block of temporary working memory */
void *_scratch_mem = NULL;
int _scratch_mem_size = 0;


/* SVGA bank switching tables */
int _last_bank_1 = -1;
int _last_bank_2 = -1;
int *_gfx_bank = NULL;

int _crtc = 0x3D4; 


/* what OS are we running under? */
int os_type = OSTYPE_UNKNOWN;

int windows_version = 0;
int windows_sub_version = 0;


/* should the timer synchronise with the screen refresh? */
int _timer_use_retrace = FALSE; 

/* vertical retrace counter and callback function */
volatile int retrace_count = 0; 
void (*retrace_proc)(void) = NULL;


/* previous signal handlers */
static void *old_sig_abrt = NULL;
static void *old_sig_fpe = NULL;
static void *old_sig_ill = NULL;
static void *old_sig_segv = NULL;
static void *old_sig_term = NULL;
static void *old_sig_int = NULL;
static void *old_sig_kill = NULL;
static void *old_sig_quit = NULL;


#define MAX_EXIT_FUNCS     16

/* we have to use a dynamic list rather than calling all the cleanup
 * routines directly, so the linker won't end up including more code than 
 * is really required.
 */
typedef void (*funcptr)(void);

static funcptr exit_funcs[MAX_EXIT_FUNCS] = 
{ 
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};



/* _add_exit_func:
 *  Adds a function to the list that need to be called by allegro_exit().
 */
void _add_exit_func(void (*func)(void))
{
   int c;

   for (c=0; c<MAX_EXIT_FUNCS; c++) {
      if ((!exit_funcs[c]) || (exit_funcs[c] == func)) {
	 exit_funcs[c] = func;
	 break;
      }
   }
}



/* _remove_exit_func:
 *  Removes a function from the list that need to be called by allegro_exit().
 */
void _remove_exit_func(void (*func)(void))
{
   int c;

   for (c=0; c<MAX_EXIT_FUNCS; c++) {
      if (exit_funcs[c] == func) {
	 while (c<MAX_EXIT_FUNCS-1) {
	    exit_funcs[c] = exit_funcs[c+1];
	    c++;
	 }
	 exit_funcs[MAX_EXIT_FUNCS-1] = NULL;
	 break;
      }
   }
}



/* signal_handler:
 *  Used to trap various signals, to make sure things get shut down cleanly.
 */
static void signal_handler(int num)
{
   static char msg[] = "Shutting down Allegro\r\n";

   allegro_exit();

   _write(STDERR_FILENO, msg, sizeof(msg)-1);

   switch (num) {

      case SIGABRT:
	 signal(num, old_sig_abrt);
	 break;

      case SIGFPE:
	 signal(num, old_sig_fpe);
	 break;

      case SIGILL:
	 signal(num, old_sig_ill);
	 break;

      case SIGSEGV:
	 signal(num, old_sig_segv);
	 break;

      case SIGTERM:
	 signal(num, old_sig_term);
	 break;

      case SIGINT:
	 signal(num, old_sig_int);
	 break;

      case SIGKILL:
	 signal(num, old_sig_kill);
	 break;

      case SIGQUIT:
	 signal(num, old_sig_quit);
	 break;

      default:
	 signal(num, SIG_DFL);
	 break;
   }

   raise(num);
}



/* allegro_init:
 *  Initialises the Allegro library. Doesn't actually do very much except
 *  setting up some variables, locking some memory, and installing 
 *  allegro_exit() as an atexit routine.
 */
int allegro_init()
{
   int i;
   char buf[16];
   __dpmi_regs r;
   union REGS regs;

   #ifndef __GNUC__
      /* call constructors manually */
      extern void initialize_datafile_types();
      extern void midi_constructor();

      initialize_datafile_types();
      midi_constructor();
   #endif

   for (i=0; i<256; i++)
      black_pallete[i] = black_rgb;

   for (i=16; i<256; i++)
      desktop_pallete[i] = desktop_pallete[i & 0x0F];

   if (inportb(0x3CC) & 1)       /* detect CRTC register address */
      _crtc = 0x3D4;
   else 
      _crtc = 0x3B4;

   #ifdef DJGPP

      os_type = OSTYPE_UNKNOWN;
      windows_version = 0;
      windows_sub_version = 0;

      /* check which OS we are running under */
      r.x.ax = 0x1600; 
      __dpmi_int(0x2F, &r);
      if ((r.h.al != 0) && (r.h.al != 1) && 
	  (r.h.al != 0x80) && (r.h.al != 0xFF)) {
	 /* win 3.1 or 95 */
	 if (r.h.al == 4) {
	    if (r.h.ah < 10)
	       os_type = OSTYPE_WIN95;
	    else
	       os_type = OSTYPE_WIN98;
	 }
	 else
	    os_type = OSTYPE_WIN3;

	 windows_version = r.h.al;
	 windows_sub_version = r.h.ah;
	 i_love_bill = TRUE;
      }
      else {
	 if (_get_dos_version(1) == 0x0532) {
	    /* win NT */
	    os_type = OSTYPE_WINNT;
	    windows_version = 0x100;
	    windows_sub_version = 0;
	    i_love_bill = TRUE;
	 }
	 else {
	    /* see if OS/2 is present */
	    r.x.ax = 0x4010;
	    __dpmi_int(0x2F, &r);
	    if (r.x.ax != 0x4010) {
	       if (r.x.ax == 0x0000) {
		  /* OS/2 Warp 3 */
		  os_type = OSTYPE_WARP;
		  i_love_bill = TRUE;
	       }
	       else {
		  /* no Warp, but previous OS/2 is available */
		  os_type = OSTYPE_OS2;
		  i_love_bill = TRUE;
	       }
	    }
	    else {
	       /* check if running under Linux DOSEMU */
	       dosmemget(0xFFFF5, 10, buf); 
	       buf[8] = 0;
	       if (!strcmp(buf, "02/25/93")) {
		  regs.x.ax = 0;
		  int86(0xE6, &regs, &regs);
		  if (regs.x.ax == 0xAA55) {
		     os_type = OSTYPE_DOSEMU;
		     windows_version = -1;
		     windows_sub_version = -1;
		     i_love_bill = TRUE;     /* (evil chortle) */
		  }
	       }
	       else {
		  /* check if running under OpenDOS */
		  r.x.ax = 0x4452;
		  __dpmi_int(0x21, &r);
		  if ((r.x.ax >= 0x1072) && !(r.x.flags & 1)) {
		     os_type = OSTYPE_OPENDOS;
		     /* now check for OpenDOS EMM386.EXE */
		     r.x.ax = 0x12FF;
		     r.x.bx = 0x0106;
		     __dpmi_int(0x2F, &r);
		     if ((r.x.ax == 0x0) && (r.x.bx == 0xEDC0))
			i_love_bill = TRUE;
		  }
	       }
	    }
	 }
      }

      _crt0_startup_flags &= ~_CRT0_FLAG_UNIX_SBRK;
      _crt0_startup_flags |= _CRT0_FLAG_NONMOVE_SBRK;

      LOCK_VARIABLE(screen);
      LOCK_VARIABLE(gfx_driver);
      LOCK_VARIABLE(gfx_capabilities);
      LOCK_VARIABLE(_current_pallete);
      LOCK_VARIABLE(_crtc);
      LOCK_VARIABLE(_drawing_mode);
      LOCK_VARIABLE(_drawing_pattern);
      LOCK_VARIABLE(_drawing_x_anchor);
      LOCK_VARIABLE(_drawing_y_anchor);
      LOCK_VARIABLE(_drawing_x_mask);
      LOCK_VARIABLE(_drawing_y_mask);
      LOCK_VARIABLE(os_type);
      LOCK_VARIABLE(windows_version);
      LOCK_VARIABLE(windows_sub_version);

   #endif   /* ifdef djgpp */

   if (_allegro_count == 0)
      atexit(allegro_exit);

   old_sig_abrt = signal(SIGABRT, signal_handler);
   old_sig_fpe  = signal(SIGFPE,  signal_handler);
   old_sig_ill  = signal(SIGILL,  signal_handler);
   old_sig_segv = signal(SIGSEGV, signal_handler);
   old_sig_term = signal(SIGTERM, signal_handler);
   old_sig_int  = signal(SIGINT,  signal_handler);
   old_sig_kill = signal(SIGKILL, signal_handler);
   old_sig_quit = signal(SIGQUIT, signal_handler);

   _load_config_text();

   _allegro_count++;

   return 0;
}



/* allegro_exit:
 *  Closes down the Allegro system. This includes restoring the initial
 *  pallete and video mode, and removing whatever mouse, keyboard, and
 *  timer routines have been installed.
 */
void allegro_exit()
{
   int c;

   for (c=MAX_EXIT_FUNCS-1; c>=0; c--)
      if (exit_funcs[c])
	 (*exit_funcs[c])();

   if (_scratch_mem) {
      free(_scratch_mem);
      _scratch_mem = NULL;
      _scratch_mem_size = 0;
   }
}



/* _vga_vsync:
 *  Waits for the start of a vertical retrace.
 */
void _vga_vsync()
{
   _vsync_out_v();
   _vsync_in();
}



/* _set_vga_virtual_width:
 *  Used by various graphics drivers to adjust the virtual width of
 *  the screen, using VGA register 0x3D4 index 0x13.
 */
void _set_vga_virtual_width(int old_width, int new_width)
{
   int width;

   if (old_width != new_width) {
      width = _read_vga_register(_crtc, 0x13);
      _write_vga_register(_crtc, 0x13, (width * new_width) / old_width);
   }
}

