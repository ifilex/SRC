/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This is a very simple program showing how to get into graphics
 *    mode and draw text onto the screen.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


int main()
{
   /* you should always do this at the start of Allegro programs */
   allegro_init();

   /* set up the keyboard handler */
   install_keyboard(); 

   /* set VGA graphics mode 13h (sized 320x200) */
   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);

   /* set the color pallete */
   set_pallete(desktop_pallete);

   /* write some text to the screen */
   textout_centre(screen, font, "Hello, world!", SCREEN_W/2, SCREEN_H/2, 255);

   /* wait for a keypress */
   readkey();

   return 0;
}
