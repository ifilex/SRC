/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program demonstrates how to access the contents of an
 *    Allegro datafile (created by the grabber utility).
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


/* the grabber produces this header, which contains defines for the names
 * of all the objects in the datafile (BIG_FONT, SILLY_BITMAP, etc).
 */
#include "example.h"



int main(int argc, char *argv[])
{
   DATAFILE *datafile;
   char buf[256];

   allegro_init();
   install_keyboard(); 
   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);

   /* load the datafile into memory */
   replace_filename(buf, argv[0], "example.dat", sizeof(buf));
   datafile = load_datafile(buf);
   if (!datafile) {
      allegro_exit();
      printf("Error loading %s!\n\n", buf);
      return 1;
   }

   /* select the pallete which was loaded from the datafile */
   set_pallete(datafile[THE_PALLETE].dat);

   /* display the bitmap from the datafile */
   textout(screen, font, "This is the bitmap:", 32, 16, 255);
   blit(datafile[SILLY_BITMAP].dat, screen, 0, 0, 64, 32, 64, 64);

   /* and use the font from the datafile */
   textout(screen, datafile[BIG_FONT].dat, "And this is a big font!", 32, 128, 96);

   readkey();

   /* unload the datafile when we are finished with it */
   unload_datafile(datafile);

   return 0;
}
