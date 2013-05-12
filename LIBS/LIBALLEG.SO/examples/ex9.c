/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program uses mode-X and moves a circle across the screen, 
 *    first with a double buffer and then using page flips. It uses 
 *    sub-bitmaps to get access to two seperate screen pages.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


int main()
{
   BITMAP *buffer;
   BITMAP *page1, *page2;
   BITMAP *active_page;
   int c;

   allegro_init();
   install_timer();

   /*  For page flipping we need a virtual screen at least twice the size
    *  of the physical one, so we request a virtual height of 400 pixels.
    */
   set_gfx_mode(GFX_MODEX, 320, 200, 0, 400);

   set_pallete(desktop_pallete);

   /* allocate the memory buffer */
   buffer = create_bitmap(SCREEN_W, SCREEN_H);

   /* first with a double buffer... */
   c = retrace_count+32;
   while (retrace_count-c <= SCREEN_W+32) {
      clear(buffer);
      circlefill(buffer, retrace_count-c, SCREEN_H/2, 32, 255);
      textout(buffer, font, "Double buffered (mode-X)", 0, 0, 255);
      blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
   }

   destroy_bitmap(buffer);

   /* now create two sub-bitmaps for page flipping */
   page1 = create_sub_bitmap(screen, 0, 0, SCREEN_W, SCREEN_H);
   page2 = create_sub_bitmap(screen, 0, SCREEN_H, SCREEN_W, SCREEN_H);
   active_page = page2;

   /* do the animation using page flips... */
   for (c=-32; c<=SCREEN_W+32; c++) {
      clear(active_page);
      circlefill(active_page, c, SCREEN_H/2, 32, 255);
      textout(active_page, font, "Page flipping (mode-X)", 0, 0, 255);

      if (active_page == page1) {
	 scroll_screen(0, 0);
	 active_page = page2;
      }
      else {
	 scroll_screen(0, SCREEN_H);
	 active_page = page1;
      }
   }

   destroy_bitmap(page1);
   destroy_bitmap(page2);

   return 0;
}
