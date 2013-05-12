/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program demonstrates the use of triple buffering and vertical
 *    retrace interrupt simulation.
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "allegro.h"


#define NUM_SHAPES   16


typedef struct SHAPE
{
   int color;                          /* color of the shape */
   fixed x, y;                         /* centre of the shape */
   fixed dir1, dir2, dir3;             /* directions to the three corners */
   fixed dist1, dist2, dist3;          /* distances to the three corners */
   fixed xc, yc, ac;                   /* position and angle change values */
} SHAPE;


SHAPE shapes[NUM_SHAPES];



/* randomly initialises a shape structure */
void init_shape(SHAPE *shape)
{
   shape->color = 1+(random()%15);

   /* randomly position the corners */
   shape->dir1 = itofix(random()%256);
   shape->dir2 = itofix(random()%256);
   shape->dir3 = itofix(random()%256);

   shape->dist1 = itofix(random()%64);
   shape->dist2 = itofix(random()%64);
   shape->dist3 = itofix(random()%64);

   /* random centre position and movement speed/direction */
   shape->x = itofix(random() % SCREEN_W);
   shape->y = itofix(random() % SCREEN_H);
   shape->ac = itofix((random()%9)-4);
   shape->xc = itofix((random()%7)-2);
   shape->yc = itofix((random()%7)-2);
}



/* updates the position of a shape structure */
void move_shape(SHAPE *shape)
{
   shape->x += shape->xc;
   shape->y += shape->yc;

   shape->dir1 += shape->ac;
   shape->dir2 += shape->ac;
   shape->dir3 += shape->ac;

   if (((shape->x <= 0) && (shape->xc < 0)) ||
       ((shape->x >= itofix(SCREEN_W)) && (shape->xc > 0))) {
      shape->xc = -shape->xc;
      shape->ac = itofix((random()%9)-4);
   }

   if (((shape->y <= 0) && (shape->yc < 0)) ||
       ((shape->y >= itofix(SCREEN_H)) && (shape->yc > 0))) {
      shape->yc = -shape->yc;
      shape->ac = itofix((random()%9)-4);
   }
}



/* draws a frame of the animation */
void draw(BITMAP *b)
{
   int c;

   clear(b);

   for (c=0; c<NUM_SHAPES; c++) {
      triangle(b, 
	       fixtoi(shapes[c].x+fmul(shapes[c].dist1, fcos(shapes[c].dir1))),
	       fixtoi(shapes[c].y+fmul(shapes[c].dist1, fsin(shapes[c].dir1))),
	       fixtoi(shapes[c].x+fmul(shapes[c].dist2, fcos(shapes[c].dir2))),
	       fixtoi(shapes[c].y+fmul(shapes[c].dist2, fsin(shapes[c].dir2))),
	       fixtoi(shapes[c].x+fmul(shapes[c].dist3, fcos(shapes[c].dir3))),
	       fixtoi(shapes[c].y+fmul(shapes[c].dist3, fsin(shapes[c].dir3))),
	       shapes[c].color);

      move_shape(shapes+c);
   } 

   textout(b, font, "Triple buffering with retrace sync", 0, 0, 255);
}



int fade_color = 63;

/* vertical retrace callback function for doing the pallete fade */
void fade()
{
   int c = (fade_color < 64) ? fade_color : 127 - fade_color;
   RGB rgb = { c, c, c, 0 };

   _set_color(0, &rgb);

   fade_color++;
   if (fade_color >= 128)
      fade_color = 0;
}

END_OF_FUNCTION(fade);



/* main animation control loop */
void triple_buffer(BITMAP *page1, BITMAP *page2, BITMAP *page3)
{
   BITMAP *active_page = page1;
   int page = 0;

   do {
      /* draw a frame */
      draw(active_page);

      /* make sure the last flip request has actually happened */
      do {
      } while (poll_scroll());

      /* post a request to display the page we just drew */
      request_scroll(0, page*SCREEN_H);

      /* update counters to point to the next page */
      switch (page) {
	 case 0:  page = 1;  active_page = page2;  break;
	 case 1:  page = 2;  active_page = page3;  break;
	 case 2:  page = 0;  active_page = page1;  break;
      }

   } while (!keypressed());

   clear_keybuf();
}



int main()
{
   BITMAP *page1, *page2, *page3;
   int c;

   srandom(time(NULL));

   allegro_init();
   install_timer();
   install_keyboard();

   if (windows_version > 0) {
      printf("\nWindows %d.%d detected. This program is unlikely to work!\n\n", windows_version, windows_sub_version);
      printf("Press a key to run it anyway...\n");
      readkey();
   }

   /*  For triple buffering we need a virtual screen at least three times
    *  the size of the physical one, so we request a virtual height of 720
    *  pixels.
    */
   set_gfx_mode(GFX_MODEX, 320, 240, 0, 720);

   set_pallete(desktop_pallete);

   /* allocate three sub bitmaps to access pages of the screen */
   page1 = create_sub_bitmap(screen, 0, 0, SCREEN_W, SCREEN_H);
   page2 = create_sub_bitmap(screen, 0, SCREEN_H, SCREEN_W, SCREEN_H);
   page3 = create_sub_bitmap(screen, 0, SCREEN_H*2, SCREEN_W, SCREEN_H);

   /* initialise the shapes */
   for (c=0; c<NUM_SHAPES; c++)
      init_shape(shapes+c);

   text_mode(-1);

   LOCK_VARIABLE(fade_color);
   LOCK_FUNCTION(fade);

   timer_simulate_retrace(TRUE);
   retrace_proc = fade;

   triple_buffer(page1, page2, page3);

   destroy_bitmap(page1);
   destroy_bitmap(page2);
   destroy_bitmap(page3);

   retrace_proc = NULL;

   return 0;
}


