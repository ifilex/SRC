/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program demonstrates the use of double buffering.
 *    It moves a circle across the screen, first just erasing and
 *    redrawing directly to the screen, then with a double buffer.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


int main()
{
   BITMAP *buffer;
   int c;

   allegro_init();
   install_timer();

   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);
   set_pallete(desktop_pallete);

   /* allocate the memory buffer */
   buffer = create_bitmap(SCREEN_W, SCREEN_H);

   /* First without any buffering...
    * Note use of the global retrace_counter to control the speed.
    */
   c = retrace_count+32;
   while (retrace_count-c <= SCREEN_W+32) {
      clear(screen);
      circlefill(screen, retrace_count-c, SCREEN_H/2, 32, 255);
      textout(screen, font, "No buffering (mode 13h)", 0, 0, 255);
   }

   /* and now with a double buffer... */
   c = retrace_count+32;
   while (retrace_count-c <= SCREEN_W+32) {
      clear(buffer);
      circlefill(buffer, retrace_count-c, SCREEN_H/2, 32, 255);
      textout(buffer, font, "Double buffered (mode 13h)", 0, 0, 255);
      blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
   }

   destroy_bitmap(buffer);

   return 0;
}
