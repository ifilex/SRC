/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program demonstrates how to manipulate the pallete. It draws
 *    a set of concentric circles onto the screen and animates them by
 *    cycling the pallete.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


int main()
{
   PALLETE pallete;
   RGB temp;
   int c;

   allegro_init();
   install_keyboard(); 
   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);

   /* first set the pallete to black to hide what we are doing */
   set_pallete(black_pallete);

   /* draw some circles onto the screen */
   for (c=255; c>0; c--)
      circlefill(screen, SCREEN_W/2, SCREEN_H/2, c, c);

   /* fill our pallete with a gradually altering sequence of colors */
   for (c=0; c<64; c++) {
      pallete[c].r = c;
      pallete[c].g = 0;
      pallete[c].b = 0;
   }
   for (c=64; c<128; c++) {
      pallete[c].r = 127-c;
      pallete[c].g = c-64;
      pallete[c].b = 0;
   }
   for (c=128; c<192; c++) {
      pallete[c].r = 0;
      pallete[c].g = 191-c;
      pallete[c].b = c-128;
   }
   for (c=192; c<256; c++) {
      pallete[c].r = 0;
      pallete[c].g = 0;
      pallete[c].b = 255-c;
   }

   /* animate the image by rotating the pallete */
   while (!keypressed()) {
      temp = pallete[255];
      for (c=255; c>0; c--)
	 pallete[c] = pallete[c-1];
      pallete[0] = temp;
      set_pallete(pallete);
   }

   return 0;
}
