/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program demonstrates how to load and display a bitmap file.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


int main(int argc, char *argv[])
{
   BITMAP *the_image;
   PALLETE the_pallete;

   if (argc != 2) {
      printf("Usage: 'ex15 filename.[bmp|lbm|pcx|tga]'\n");
      return 1;
   }

   /* read in the bitmap file */
   the_image = load_bitmap(argv[1], the_pallete);
   if (!the_image) {
      printf("Error reading bitmap file '%s'\n", argv[1]);
      return 1;
   }

   allegro_init();
   install_keyboard(); 
   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);

   /* select the bitmap pallete */
   set_pallete(the_pallete);

   /* blit the image onto the screen */
   blit(the_image, screen, 0, 0, (SCREEN_W-the_image->w)/2, 
		     (SCREEN_H-the_image->h)/2, the_image->w, the_image->h);

   /* destroy the bitmap */
   destroy_bitmap(the_image);

   readkey();
   return 0;
}
