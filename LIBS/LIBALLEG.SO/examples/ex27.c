/*
 *      Example program for the Allegro library, by Grzegorz Adam Hankiewicz
 *
 *      This program uses the Allegro library to detect and read the value
 *      of a joystick. The output of the program is a small target sight
 *      on the screen which you can move.
 *      At the same time the program will tell you what you are doing with
 *      the joystick (moving or firing).
 */


#include <stdio.h>

#include "allegro.h"


int main()
{
   BITMAP *bmp;            /* we create a pointer to a virtual screen */
   int x=160, y=100;       /* these will be used to show the target sight */
   int analogmode;
   char *msg;
   int c;

   allegro_init();         /* you NEED this man! ;-) */

   install_keyboard();     /* ahh... read the docs. I will explain only
			    * joystick specific routines
			    */

   printf("\nWelcome to the Joystick test.\n");
   printf("\nPlease center the joystick and press a key.\n");

   if ((readkey()&0xFF) == 27)
      return 0;

   /* the first thing is to initialise the joystick driver */
   if (install_joystick(JOY_TYPE_AUTODETECT) != 0) {
      printf("\nError initialising joystick\n%s\n\n", allegro_error);
      return 1;
   }

   /* make sure that we really do have a joystick */
   if (joy_type == JOY_TYPE_NONE) {
      printf("\nError: joystick not found\n\n");
      return 1;
   }

   /* before using the joystick, we have to calibrate it. This loop only
    * calibrates joystick number 0, but you could do the same thing for
    * other sticks if they are present (the num_joysticks variable will 
    * tell you how many there are).
    */
   while (joy[0].flags & JOYFLAG_CALIBRATE) {
      msg = calibrate_joystick_name(0);
      printf("%s and press a key.\n", msg);

      if ((readkey()&0xFF) == 27)
	 return 0;

      if (calibrate_joystick(0) != 0) {
	 printf("\nError calibrating joystick!\n\n");
	 return 1;
      }
   }

   printf("\n%s has been initialised and calibrated.\n\n", joystick_driver->name);

   /* if this joystick supports analogue input, ask the user whether to
    * use digital or analogue mode. If it is only a digital pad, we don't
    * bother with this question.
   */
   if (joy[0].stick[0].flags & JOYFLAG_ANALOGUE) {
      printf("Now press 'D' to use a digital joystick or 'A' for analogue mode.\n");

      for (;;) {
	 c = readkey()&0xFF;

	 if ((c=='d') || (c=='D')) {
	    analogmode = FALSE;
	    break;
	 }
	 else if ((c=='a') || (c=='A')) {
	    analogmode = TRUE;
	    break;
	 }
	 else if (c == 27)
	    return 0;
      }
   }
   else
      analogmode = FALSE;

   set_gfx_mode(GFX_AUTODETECT, 320, 200, 0, 0);
   drawing_mode(DRAW_MODE_XOR, 0, 0, 0);
   clear_keybuf();

   bmp = create_bitmap(320, 200);
   clear(bmp);

   do {
      poll_joystick();     /* we HAVE to do this to read the joystick */

      clear(bmp);

      if (analogmode)
	 textout_centre(bmp, font, "Analog Joystick selected", 160, 160, 15);
      else
	 textout_centre(bmp, font, "Digital Joystick selected", 160, 160, 15);

      textout_centre(bmp, font, "Move the Joystick all around", 160, 170, 15);
      textout_centre(bmp, font, "Press any key to exit", 160, 180, 15);
      textout_centre(bmp, font, "Made by Grzegorz Adam Hankiewicz,1997", 160, 190, 15);

      /* if we detect any buttons, we print a message on the screen */
      for (c=0; c<joy[0].num_buttons; c++)
	 if (joy[0].button[c].b)
	    textprintf_centre(bmp, font, 160, c*10, 15, "%s pressed", joy[0].button[c].name);

      if (!analogmode) {
	 /* now we have to check individually every possible movement
	  * and actualize the coordinates of the target sight.
	  */
	 if (joy[0].stick[0].axis[0].d1) {
	    if (x > 0)
	       x--;
	    textout_centre(bmp, font, "Left", 120, 100, 15);
	 }
	 if (joy[0].stick[0].axis[0].d2) {
	    if (x < 319)
	       x++;
	    textout_centre(bmp, font, "Right", 200, 100, 15);
	 }
	 if (joy[0].stick[0].axis[1].d1) {
	    if (y > 0)
	       y--;
	    textout_centre(bmp, font, "Up", 160, 70, 15);
	 }
	 if (joy[0].stick[0].axis[1].d2) {
	    if (y < 199)
	       y++;
	    textout_centre(bmp, font, "Down", 160, 130, 15);
	 }
      }
      else {
	 /* yeah! Remember the 'ifs' of the digital part? This looks
	  * much better, only 2 lines.
	  */
	 x += joy[0].stick[0].axis[0].pos/40;
	 y += joy[0].stick[0].axis[1].pos/40;

	 /* by checking if the values were positive or negative, we
	  * can know in which the direction the user pulled the joy.
	  */
	 if (joy[0].stick[0].axis[0].pos/40 < 0) 
	    textout_centre(bmp, font, "Left", 120, 100, 15);

	 if (joy[0].stick[0].axis[0].pos/40 > 0) 
	    textout_centre(bmp, font, "Right", 200, 100, 15);

	 if (joy[0].stick[0].axis[1].pos/40 < 0) 
	    textout_centre(bmp, font, "Up", 160, 70, 15);

	 if (joy[0].stick[0].axis[1].pos/40 > 0) 
	    textout_centre(bmp, font, "Down", 160, 130, 15);

	 /* WARNING! An analog joystick can move more than 1 pixel at
	  * a time and the checks we did with the digital part don't
	  * work any longer because the steps of the target sight could
	  * 'jump' over the limits.
	  * To avoid this, we just check if the target sight has gone
	  * out of the screen. If yes, we put it back at the border.
	  */
	 if (x>319) x=319;
	 if (x<0)   x=0;
	 if (y<0)   y=0;
	 if (y>199) y=199;
      }

      /* this draws the target sight. */
      circle(bmp, x, y, 5, 15);
      putpixel(bmp, x, y, 15);
      putpixel(bmp, x+1, y, 15);
      putpixel(bmp, x, y+1, 15);
      putpixel(bmp, x-1, y, 15);
      putpixel(bmp, x, y-1, 15);
      putpixel(bmp, x+5, y, 15);
      putpixel(bmp, x, y+5, 15);
      putpixel(bmp, x-5, y, 15);
      putpixel(bmp, x, y-5, 15);

      blit(bmp, screen, 0, 0, 0, 0, 320, 200);

   } while (!keypressed());

   /* if you press a key, you will be again in the peaceful DOS. */
   return 0;
}
