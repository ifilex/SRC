/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program demonstrates how to get mouse input.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


int main()
{
   int mickeyx = 0;
   int mickeyy = 0;
   BITMAP *custom_cursor;
   char msg[80];
   int c = 0;

   allegro_init();
   install_keyboard(); 
   install_mouse();
   install_timer();
   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);
   set_pallete(desktop_pallete);

   do {
      /* the mouse position is stored in the variables mouse_x and mouse_y */
      sprintf(msg, "mouse_x = %-5d", mouse_x);
      textout(screen, font, msg, 16, 16, 255);

      sprintf(msg, "mouse_y = %-5d", mouse_y);
      textout(screen, font, msg, 16, 32, 255);

      /* or you can use this function to measure the speed of movement.
       * Note that we only call it every fourth time round the loop: 
       * there's no need for that other than to slow the numbers down 
       * a bit so that you will have time to read them...
       */
      c++;
      if ((c & 3) == 0)
	 get_mouse_mickeys(&mickeyx, &mickeyy);

      sprintf(msg, "mickey_x = %-7d", mickeyx);
      textout(screen, font, msg, 16, 64, 255);

      sprintf(msg, "mickey_y = %-7d", mickeyy);
      textout(screen, font, msg, 16, 80, 255);

      /* the mouse button state is stored in the variable mouse_b */
      if (mouse_b & 1)
	 textout(screen, font, "left button is pressed ", 16, 112, 255);
      else
	 textout(screen, font, "left button not pressed", 16, 112, 255);

      if (mouse_b & 2)
	 textout(screen, font, "right button is pressed ", 16, 128, 255);
      else
	 textout(screen, font, "right button not pressed", 16, 128, 255);

      if (mouse_b & 4)
	 textout(screen, font, "middle button is pressed ", 16, 144, 255);
      else
	 textout(screen, font, "middle button not pressed", 16, 144, 255);

      vsync();

   } while (!keypressed());

   clear_keybuf();

   /*  To display a mouse pointer, call show_mouse(). There are several 
    *  things you should be aware of before you do this, though. For one,
    *  it won't work unless you call install_timer() first. For another,
    *  you must never draw anything onto the screen while the mouse
    *  pointer is visible. So before you draw anything, be sure to turn 
    *  the mouse off with show_mouse(NULL), and turn it back on again when
    *  you are done.
    */
   clear(screen);
   textout_centre(screen, font, "Press a key to change cursor", SCREEN_W/2, SCREEN_H/2, 255);
   show_mouse(screen);
   readkey();
   show_mouse(NULL);

   /* create a custom mouse cursor bitmap... */
   custom_cursor = create_bitmap(32, 32);
   clear(custom_cursor); 
   for (c=0; c<8; c++)
      circle(custom_cursor, 16, 16, c*2, c);

   /* select the custom cursor and set the focus point to the middle of it */
   set_mouse_sprite(custom_cursor);
   set_mouse_sprite_focus(16, 16);

   clear(screen);
   textout_centre(screen, font, "Press a key to quit", SCREEN_W/2, SCREEN_H/2, 255);
   show_mouse(screen);
   readkey();
   show_mouse(NULL);

   destroy_bitmap(custom_cursor);

   return 0;
}
