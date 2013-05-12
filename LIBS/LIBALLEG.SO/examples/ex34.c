/* 
 * Example program for the Allegro library, by Grzegorz Adam Hankiewicz.
 *
 * This program demonstrates how to access the contents of an Allegro
 * datafile (created by the grabber utility) linked to the exe by the
 * exedat tool. It is based on the ex12 example.
 *
 * You may ask: how do you compile, append and exec your program?
 *
 * Answer: like this...
 *
 *    1) Compile your program like normal. Use the magic filenames with '#'
 *       to load your data where needed.
 *
 *    2) Now, if you have DJP, it's time for using it. Run "djp foo.exe"
 *
 *    3) Once you compressed your program, run "exedat foo.exe data.dat"
 *
 *    4) Finally run your program.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


/* the grabber produces this header, which contains defines for the names
 * of all the objects in the datafile (BIG_FONT, SILLY_BITMAP, etc).
 * We still need to keep this, since we want to know the names of the objects.
 */
#include "example.h"


int main(int argc, char *argv[])
{
   DATAFILE *datafile;

   allegro_init();
   install_keyboard(); 
   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);

   /* load the datafile into memory */
   datafile = load_datafile("#");
   if (!datafile) {
      allegro_exit();
      printf("Unable to load the appended datafile!\n\n");
      printf("This program reads graphics from the end of the executable file.\n");
      printf("Before running it, you must append this data with the exedat utility.\n\n");
      printf("Example command line:\n\n");
      printf("\texedat ex34.exe example.dat\n\n");
      printf("To compress the appended data, pass the -c switch to exedat.\n\n");
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
