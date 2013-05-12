/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program demonstrates how to use hardware scrolling and split
 *    screens in mode-X.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


int main()
{
   BITMAP *scroller, *status_bar;
   int counter = 0;
   char tmp[80];
   RGB black = { 0, 0, 0, 0 };
   int x = 0;
   int next_x;
   int h = 100;

   allegro_init();
   install_keyboard();

   /* create a nice wide virtual screen, and split it at line 200 */
   set_gfx_mode(GFX_MODEX, 320, 240, 640, 240);
   split_modex_screen(200);
   scroll_screen(0, 40);

   /* the scrolling area is sized 640x200 and starts at line 40 */
   scroller = create_sub_bitmap(screen, 0, 40, 640, 200);

   /* the status bar is sized 320x40 and starts at line 0 */
   status_bar = create_sub_bitmap(screen, 0, 0, 320, 40);

   set_pallete(desktop_pallete);
   set_color(0, &black);

   textout(status_bar, font, "This area isn't scrolling", 8, 8, 1);

   rectfill(scroller, 0, 0, 320, 100, 6);
   rectfill(scroller, 0, 100, 320, 200, 2);

   do {
      /* update the status bar */
      sprintf(tmp, "Counter = %d", counter++);
      textout(status_bar, font, tmp, 8, 20, 1);

      /* advance the scroller, wrapping every 320 pixels */
      next_x = x + 1;
      if (next_x >= 320)
	 next_x = 0;

      /* draw another column of the landscape */
      vline(scroller, next_x+319, 0, h, 6);
      vline(scroller, next_x+319, h+1, 200, 2);

      /* scroll the screen */
      scroll_screen(next_x, 40);

      /* duplicate the landscape column so we can wrap the scroller */
      if (next_x > 0) {
	 vline(scroller, x, 0, h, 6);
	 vline(scroller, x, h+1, 200, 2);
      }

      /* randomly alter the landscape position */
      if (random()&1) {
	 if (h > 5)
	    h--;
      }
      else {
	 if (h < 195)
	    h++;
      }

      x = next_x;

   } while (!keypressed());

   destroy_bitmap(scroller);
   destroy_bitmap(status_bar);

   return 0;
}
