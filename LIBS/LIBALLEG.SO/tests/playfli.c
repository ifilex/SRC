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
 *      FLI player test program for the Allegro library.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "allegro.h"


#ifdef __GNUC__
void usage() __attribute__ ((noreturn));
#endif


void usage()
{
   printf("\nFLI player test program for Allegro " ALLEGRO_VERSION_STR);
   printf("\nBy Shawn Hargreaves, " ALLEGRO_DATE_STR "\n\n");
   printf("Usage: playfli [options] filename.(fli|flc)\n\n");
   printf("Options:\n");
   printf("\t'-loop' cycles the animation until a key is pressed\n");
   printf("\t'-step' selects single-step mode\n");
   printf("\t'-mode screen_w screen_h' sets the screen mode\n");

   exit(1);
}



int loop = FALSE;
int step = FALSE;



int key_checker()
{
   if (step) {
      if ((readkey() & 0xFF) == 27)
	 return 1;
      else
	 return 0;
   }
   else {
      if (keypressed())
	 return 1;
      else
	 return 0;
   }
}



int main(int argc, char *argv[])
{
   int w = 320;
   int h = 200;
   int c, ret;

   if (argc < 2)
      usage();

   for (c=1; c<argc-1; c++) {
      if (stricmp(argv[c], "-loop") == 0)
	 loop = TRUE;
      else if (stricmp(argv[c], "-step") == 0)
	 step = TRUE;
      else if ((stricmp(argv[c], "-mode") == 0) && (c<argc-3)) {
	 w = atoi(argv[c+1]);
	 h = atoi(argv[c+2]);
	 c += 2;
      }
      else
	 usage();
   }

   allegro_init();
   install_keyboard();
   install_timer();

   if (set_gfx_mode(GFX_AUTODETECT, w, h, 0, 0) != 0) {
      allegro_exit();
      printf("\nError setting graphics mode\n%s\n", allegro_error);
      return 1;
   }

   ret = play_fli(argv[c], screen, loop, key_checker);

   allegro_exit();

   if (ret < 0)
      printf("Error playing FLI file\n");

   return 0;
}

