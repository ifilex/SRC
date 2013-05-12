/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Joystick driver for the Wingman Warrior.
 *
 *      By Kester Maddock
 *
 *      TODO: Support all SWIFT devices.  x and y values are correct,
 *      my Wingman always has these set to 0, joystick values in
 *      pitch and roll.  Joystick values range from -8192 to 8192, I
 *      >> by 6 to get -128 to 128 Allegro range.  Spinner's range is
 *      +/- 18?  I would recommend setting x/y values to hat switch's
 *      axis, and reading the hat switch as buttons instead of an axis.
 *
 *      See readme.txt for copyright information.
 */


#ifndef DJGPP
#error This file should only be used by the djgpp version of Allegro
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dpmi.h>
#include <go32.h>

#include <sys/movedata.h>

#include "allegro.h"



/* driver functions */
static int ww_init(void);
static void ww_exit(void);
static int ww_poll(void);



typedef union {
   struct {
      unsigned char type;
   } swift;
   struct {
      short x;                              // unused
      short y;                              // unused
      short z;                              // throttle
      short pitch;                          // stick y axis
      short roll;                           // stick x axis
      short yaw;                            // spinner
      short buttons;
   } data;
} swift_t;



static char ww_desc[80] = "not initialised.";



JOYSTICK_DRIVER joystick_ww =
{
   JOY_TYPE_WINGWARRIOR,
   "Wingman Warrior",
   ww_desc,
   ww_init,
   ww_exit,
   ww_poll,
   NULL, NULL,
   NULL, NULL
};


static swift_t *wingman = NULL;



static int ww_init(void)
{
   static char x[] = "X";
   static char y[] = "Y";
   static char *buttons[] = { "B1", "B2", "B3", "B4", "East", "South", "West", "North"};
   char *joy_name = get_config_text("Stick");
   char *t_name = get_config_text("Throttle");
   char *s_name = get_config_text("Spinner");
   char *hat_name = get_config_text("Hat");
   __dpmi_regs r;
   int b;

   wingman = (swift_t *) malloc(sizeof(swift_t));
   memset(wingman, 0, sizeof(swift_t));

   memset(&r, 0, sizeof(r));
   r.d.eax = 0x53c1;
   r.x.es = __tb >> 4;
   r.d.edx = 0;
   __dpmi_int(0x33, &r);

   dosmemget(__tb, sizeof(swift_t), wingman);

   if ((short) r.x.ax != 1) {
      /* SWIFT functions not present */
      sprintf(allegro_error, "SWIFT Device not detected (AX=%04x).\n",
	      (unsigned) (short) r.d.eax);
      free(wingman);
      wingman = NULL;
      return -1;
   }
   else if (wingman->swift.type != 1) {
      /* no SWIFT device, or not Wingman */
      if (wingman->swift.type == 0) {
	 sprintf(allegro_error, "Wingman Warrior not connected.\n");
	 free(wingman);
	 wingman = NULL;
	 return -1;
      }
      else {
	 sprintf(allegro_error, "Device connected is not a Wingman Warrior! (type=%d)\n",
		 wingman->swift.type);
	 free(wingman);
	 wingman = NULL;
	 return -1;
      }
   }

   sprintf(ww_desc, "Wingman Warrior connected.\n");

   num_joysticks = 1;
   joy[0].flags = JOYFLAG_ANALOGUE | JOYFLAG_SIGNED;
   joy[0].num_sticks = 3;
   joy[0].num_buttons = 8;

   joy[0].stick[0].flags = JOYFLAG_ANALOGUE | JOYFLAG_SIGNED;
   joy[0].stick[0].num_axis = 2;
   joy[0].stick[0].name = joy_name;
   joy[0].stick[0].axis[0].name = x;
   joy[0].stick[0].axis[1].name = y;

   joy[0].stick[1].flags = JOYFLAG_ANALOGUE | JOYFLAG_SIGNED;
   joy[0].stick[1].num_axis = 1;
   joy[0].stick[1].name = s_name;
   joy[0].stick[1].axis[0].name = s_name;

   joy[0].stick[2].flags = JOYFLAG_ANALOGUE | JOYFLAG_UNSIGNED;
   joy[0].stick[2].num_axis = 1;
   joy[0].stick[2].name = t_name;
   joy[0].stick[2].axis[0].name = t_name;

   for (b=0; b<4; b++)
      joy[0].button[b].name = buttons[b];

   for (b=0; b<4; b++)
      joy[0].button[b].name = get_config_text(buttons[b]);

   return 0;
}



static int ww_poll(void)
{
   __dpmi_regs r = {{0, 0, 0, 0, 0, 0, 0, 0}};

   /* No wingman for you */
   if (!wingman)
      return -1;

   /* Read from the mouse driver */
   r.d.eax = 0x5301;
   r.x.es = __tb >> 4;
   r.d.edx = 0;
   __dpmi_int(0x33, &r);
   dosmemget(__tb, sizeof(swift_t), wingman);

   /* Main X/Y Axis */
   joy[0].stick[0].axis[0].pos = -wingman->data.roll >> 6;
   joy[0].stick[0].axis[1].pos = wingman->data.pitch >> 6;

   /* Spin control */
   joy[0].stick[1].axis[0].pos = -wingman->data.yaw * 7;

   /* Throttle */
   joy[0].stick[2].axis[0].pos = (wingman->data.z >> 6) + 128;

   /* Setup Digital controls */
   joy[0].stick[0].axis[0].d1 = joy[0].stick[0].axis[0].pos < -64;
   joy[0].stick[0].axis[0].d2 = joy[0].stick[0].axis[0].pos > 64;
   joy[0].stick[0].axis[1].d1 = joy[0].stick[0].axis[1].pos < -64;
   joy[0].stick[0].axis[1].d2 = joy[0].stick[0].axis[1].pos > 64;

   joy[0].stick[1].axis[0].d1 = joy[0].stick[1].axis[0].pos < -64;
   joy[0].stick[1].axis[0].d2 = joy[0].stick[1].axis[0].pos > 64;

   /* Setup buttons */
   joy[0].button[0].b = wingman->data.buttons & 0x4;
   joy[0].button[1].b = wingman->data.buttons & 0x1;
   joy[0].button[2].b = wingman->data.buttons & 0x2;
   joy[0].button[3].b = wingman->data.buttons & 0x10;

   /* Hat Switch */
   joy[0].button[4].b = wingman->data.buttons & 0x40;
   joy[0].button[5].b = wingman->data.buttons & 0x100;
   joy[0].button[6].b = wingman->data.buttons & 0x20;
   joy[0].button[7].b = wingman->data.buttons & 0x80;

   return 0;
}



static void ww_exit(void)
{
   if (wingman) {
      free(wingman);
      wingman = NULL;
   }
}
