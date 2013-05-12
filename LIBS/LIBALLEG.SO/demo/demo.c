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
 *      A simple game demonstrating the use of the Allegro library.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#ifdef DJGPP
#include <conio.h>
#endif

#include "allegro.h"
#include "demo.h"


#define DOUBLE_BUFFER      1
#define PAGE_FLIP          2
#define RETRACE_FLIP       3
#define TRIPLE_BUFFER      4
#define DIRTY_RECTANGLE    5

int animation_type = 0;


int cheat = FALSE;
int turbo = FALSE;

DATAFILE *data;

BITMAP *s;

BITMAP *page1, *page2, *page3;
int current_page = 0;

int use_retrace_proc = FALSE;

PALLETE title_pallete;

volatile int score;
char score_buf[80];

int dead;
int pos;
int xspeed, yspeed, ycounter;
int ship_state;
int ship_burn;
int shot;
int skip_speed, skip_count;
volatile int frame_count, fps;
volatile int game_time;

#define MAX_SPEED       32
#define SPEED_SHIFT     3

#define MAX_STARS       128

volatile struct {
   fixed x, y, z;
   int ox, oy;
} star[MAX_STARS];

#define MAX_ALIENS      50

volatile struct {
   int x, y;
   int d;
   int state;
   int shot;
} alien[MAX_ALIENS];

volatile int alien_count;
volatile int new_alien_count;

#define EXPLODE_FLAG    100
#define EXPLODE_FRAMES  64
#define EXPLODE_SIZE    80

RLE_SPRITE *explosion[EXPLODE_FRAMES];

#define BULLET_SPEED    6

volatile int bullet_flag;
volatile int bullet_x, bullet_y; 

typedef struct RECT {
   int x, y, w, h;
} RECT;

typedef struct DIRTY_LIST {
   int count;
   RECT rect[2*(MAX_ALIENS+MAX_STARS+4)];
} DIRTY_LIST;

DIRTY_LIST dirty, old_dirty;


#define PAN(x)       (((x) * 256) / SCREEN_W)



void game_timer()
{
   game_time++;
}

END_OF_FUNCTION(game_timer);



void fps_proc()
{
   fps = frame_count;
   frame_count = 0;
   score++;
}

END_OF_FUNCTION(fps_proc);



void move_everyone()
{
   int c;

   /* player dead? */
   if (shot) {
      if (skip_count <= 0) {
	 if (ship_state >= EXPLODE_FLAG+EXPLODE_FRAMES-1)
	    dead = TRUE;
	 else
	    ship_state++;
	 if (yspeed)
	    yspeed--;
      }
   }
   else { 
      if (skip_count <= 0) {
	 /* moving left */
	 if ((key[KEY_LEFT]) || (joy[0].stick[0].axis[0].d1)) {
	    if (xspeed > -MAX_SPEED)
	       xspeed -= 2;
	 }
	 else {
	    /* moving right */
	    if ((key[KEY_RIGHT]) || (joy[0].stick[0].axis[0].d2)) {
	       if (xspeed < MAX_SPEED)
		  xspeed += 2;
	    }
	    else {
	       /* decelerate */
	       if (xspeed > 0)
		  xspeed -= 2;
	       else if (xspeed < 0)
		  xspeed += 2;
	    }
	 }
      }

      /* which ship sprite to use? */
      if (xspeed < -24)
	 ship_state = SHIP1;
      else if (xspeed < -2)
	 ship_state = SHIP2;
      else if (xspeed <= 2)
	 ship_state = SHIP3;
      else if (xspeed <= 24)
	 ship_state = SHIP4;
      else
	 ship_state = SHIP5;

      /* move player */ 
      pos += xspeed;
      if (pos < (32 << SPEED_SHIFT)) {
	 pos = (32 << SPEED_SHIFT);
	 xspeed = 0;
      }
      else
	 if (pos >= ((SCREEN_W-32) << SPEED_SHIFT)) {
	    pos = ((SCREEN_W-32) << SPEED_SHIFT);
	    xspeed = 0;
	 }

      if (skip_count <= 0) {
	 /* fire thrusters? */
	 if ((key[KEY_UP]) || (joy[0].stick[0].axis[1].d1) || (turbo)) {
	    if (yspeed < MAX_SPEED) {
	       if (yspeed == 0) {
		  play_sample(data[ENGINE_SPL].dat, 0, PAN(pos>>SPEED_SHIFT),
			      1000, TRUE);
	       }
	       else
		  /* fade in sample while speeding up */
		  adjust_sample(data[ENGINE_SPL].dat, yspeed*64/MAX_SPEED, 
				PAN(pos>>SPEED_SHIFT), 1000, TRUE);

	       yspeed++;
	    }
	    else
	       /* adjust pan while the sample is looping */
	       adjust_sample(data[ENGINE_SPL].dat, 64, PAN(pos>>SPEED_SHIFT),
			     1000, TRUE);

	    ship_burn = TRUE;
	    score++;
	 }
	 else {
	    if (yspeed) {
	       yspeed--;
	       if (yspeed == 0)
		  stop_sample(data[ENGINE_SPL].dat);
	       else
		  /* fade out and reduce frequency when slowing down */
		  adjust_sample(data[ENGINE_SPL].dat, yspeed*64/MAX_SPEED, 
		     PAN(pos>>SPEED_SHIFT), 500+yspeed*500/MAX_SPEED, TRUE);
	    }
	    ship_burn = FALSE;
	 }
      }
   }

   /* going fast? move everyone else down to compensate */
   if (yspeed) {
      ycounter += yspeed;
      while (ycounter >= (1 << SPEED_SHIFT)) {
	 if (bullet_flag)
	    bullet_y++;
	 for (c=0; c<MAX_STARS; c++)
	    if (++star[c].oy >= SCREEN_H)
	       star[c].oy = 0;
	 for (c=0; c<alien_count; c++)
	    alien[c].y++;
	 ycounter -= (1 << SPEED_SHIFT);
      }
   }

   /* move bullet */
   if (bullet_flag) {
      bullet_y -= BULLET_SPEED;
      if (bullet_y < 8)
	 bullet_flag = FALSE;
      else {
	 /* shot an alien? */
	 for (c=0; c<alien_count; c++) {
	    if ((ABS(bullet_y - alien[c].y) < 20) &&
		(ABS(bullet_x - alien[c].x) < 20) &&
		(!alien[c].shot)) {
	       alien[c].shot = TRUE;
	       alien[c].state = EXPLODE_FLAG;
	       bullet_flag = FALSE;
	       score += 10;
	       play_sample(data[BOOM_SPL].dat, 255, PAN(bullet_x), 1000, FALSE);
	       break; 
	    }
	 }
      }
   }

   /* move stars */
   for (c=0; c<MAX_STARS; c++) {
      if ((star[c].oy += ((int)star[c].z>>1)+1) >= SCREEN_H)
	 star[c].oy = 0;
   }

   /* fire bullet? */
   if (!shot) {
      if ((key[KEY_SPACE]) || (joy[0].button[0].b) || (joy[0].button[1].b)) {
	 if (!bullet_flag) {
	    bullet_x = (pos>>SPEED_SHIFT)-2;
	    bullet_y = SCREEN_H-64;
	    bullet_flag = TRUE;
	    play_sample(data[SHOOT_SPL].dat, 100, PAN(bullet_x), 1000, FALSE);
	 } 
      }
   }

   /* move aliens */
   for (c=0; c<alien_count; c++) {
      if (alien[c].shot) {
	 /* dying alien */
	 if (skip_count <= 0) {
	    if (alien[c].state < EXPLODE_FLAG+EXPLODE_FRAMES-1) {
	       alien[c].state++;
	       if (alien[c].state & 1)
		  alien[c].x += alien[c].d;
	    }
	    else {
	       alien[c].x = 16 + random() % (SCREEN_W-32);
	       alien[c].y = -60 - (random() & 0x3f);
	       alien[c].d = (random()&1) ? 1 : -1;
	       alien[c].shot = FALSE;
	       alien[c].state = -1;
	    }
	 }
      }
      else {
	 /* move alien sideways */
	 alien[c].x += alien[c].d;
	 if (alien[c].x < -60)
	    alien[c].x = SCREEN_W;
	 else if (alien[c].x > SCREEN_W)
	    alien[c].x = -60;
      }
      /* move alien vertically */
      alien[c].y += 1;
      if (alien[c].y > SCREEN_H+30) {
	 if (!alien[c].shot) {
	    alien[c].x = random() % (SCREEN_W-32);
	    alien[c].y = -32 - (random() & 0x3f);
	    alien[c].d = (random()&1) ? 1 : -1;
	 }
      }
      else {
	 /* alien collided with player? */
	 if ((ABS(alien[c].x - (pos>>SPEED_SHIFT)) < 48) &&
	     (ABS(alien[c].y - (SCREEN_H-42)) < 32)) {
	    if (!shot) {
	       if (!cheat) {
		  ship_state = EXPLODE_FLAG;
		  shot = TRUE;
		  stop_sample(data[ENGINE_SPL].dat);
	       }
	       if ((!cheat) || (!alien[c].shot))
		  play_sample(data[DEATH_SPL].dat, 255, PAN(pos>>SPEED_SHIFT), 1000, FALSE);
	    }
	    if (!alien[c].shot) {
	       alien[c].shot = TRUE;
	       alien[c].state = EXPLODE_FLAG;
	    }
	 }
      }
   }

   if (skip_count <= 0) {
      skip_count = skip_speed;

      /* make a new alien? */
      new_alien_count++;
      if ((new_alien_count > 600) && (alien_count < MAX_ALIENS)) {
	 alien_count++;
	 new_alien_count = 0;
      }
   }
   else
      skip_count--;

   /* Yes, I know that isn't a very pretty piece of code :-) */
}



void add_to_list(DIRTY_LIST *list, int x, int y, int w, int h)
{
   list->rect[list->count].x = x;
   list->rect[list->count].y = y;
   list->rect[list->count].w = w;
   list->rect[list->count].h = h;
   list->count++; 
}



int rect_cmp(const void *p1, const void *p2)
{
   RECT *r1 = (RECT *)p1;
   RECT *r2 = (RECT *)p2;

   return (r1->y - r2->y);
}



void draw_screen()
{
   int c;
   int i, j;
   int x, y;
   BITMAP *bmp;
   RLE_SPRITE *spr;
   char *animation_type_str;

   if (animation_type == DOUBLE_BUFFER) {
      /* for double buffering, draw onto the memory bitmap. The first step 
       * is to clear it...
       */
      animation_type_str = "double buffered";
      bmp = s;
      clear(bmp);
   }
   else if ((animation_type == PAGE_FLIP) || (animation_type == RETRACE_FLIP)) {
      /* for page flipping we draw onto one of the sub-bitmaps which
       * describe different parts of the large virtual screen.
       */ 
      if (animation_type == PAGE_FLIP) 
	 animation_type_str = "page flipping";
      else
	 animation_type_str = "synced flipping";

      if (current_page == 0) {
	 bmp = page2;
	 current_page = 1;
      }
      else {
	 bmp = page1;
	 current_page = 0;
      }
      clear(bmp);
   }
   else if (animation_type == TRIPLE_BUFFER) {
      /* triple buffering works kind of like page flipping, but with three
       * pages (obvious, really :-) The benefit of this is that we can be
       * drawing onto page a, while displaying page b and waiting for the
       * retrace that will flip across to page c, hence there is no need
       * to waste time sitting around waiting for retraces.
       */ 
      animation_type_str = "triple buffered";

      if (current_page == 0) {
	 bmp = page2; 
	 current_page = 1;
      }
      else if (current_page == 1) {
	 bmp = page3; 
	 current_page = 2; 
      }
      else {
	 bmp = page1; 
	 current_page = 0;
      }
      clear(bmp);
   }
   else {
      /* for dirty rectangle animation we draw onto the memory bitmap, but 
       * we can use information saved during the last draw operation to
       * only clear the areas that have something on them.
       */
      animation_type_str = "dirty rectangles";
      bmp = s;

      for (c=0; c<dirty.count; c++) {
	 if ((dirty.rect[c].w == 1) && (dirty.rect[c].h == 1))
	    putpixel(bmp, dirty.rect[c].x, dirty.rect[c].y, 0);
	 else
	    rectfill(bmp, dirty.rect[c].x, dirty.rect[c].y, 
			dirty.rect[c].x + dirty.rect[c].w, 
			dirty.rect[c].y+dirty.rect[c].h, 0);
      }

      old_dirty = dirty;
      dirty.count = 0;
   }

   /* draw the stars */
   for (c=0; c<MAX_STARS; c++) {
      y = star[c].oy;
      x = ((star[c].ox-SCREEN_W/2)*(y/(4-star[c].z/2)+SCREEN_H)/SCREEN_H)+SCREEN_W/2;

      putpixel(bmp, x, y, 15-(int)star[c].z);

      if (animation_type == DIRTY_RECTANGLE)
	 add_to_list(&dirty, x, y, 1, 1);
   }

   /* draw the player */
   x = pos>>SPEED_SHIFT;

   if ((ship_burn) && (ship_state < EXPLODE_FLAG)) {
      spr = (RLE_SPRITE *)data[ENGINE1+(retrace_count/4)%7].dat;
      draw_rle_sprite(bmp, spr, x-spr->w/2, SCREEN_H-24); 

      if (animation_type == DIRTY_RECTANGLE)
	 add_to_list(&dirty, x-spr->w/2, SCREEN_H-24, spr->w, spr->h);
   }

   if (ship_state >= EXPLODE_FLAG)
      spr = explosion[ship_state-EXPLODE_FLAG];
   else
      spr = (RLE_SPRITE *)data[ship_state].dat;

   draw_rle_sprite(bmp, spr, x-spr->w/2, SCREEN_H-42-spr->h/2); 

   if (animation_type == DIRTY_RECTANGLE)
      add_to_list(&dirty, x-spr->w/2, SCREEN_H-42-spr->h/2, spr->w, spr->h);

   /* draw the aliens */
   for (c=0; c<alien_count; c++) {
      x = alien[c].x;
      y = alien[c].y;

      if (alien[c].state >= EXPLODE_FLAG) {
	 spr = explosion[alien[c].state-EXPLODE_FLAG];
      }
      else {
	 switch (c%3) {
	    case 0: i = ASTA01; break;
	    case 1: i = ASTB01; break;
	    case 2: i = ASTC01; break;
	    default: i = 0; break;
	 }
	 j = (retrace_count/(6-(c&3))+c)%15;
	 if (c&1)
	    spr = (RLE_SPRITE *)data[i+14-j].dat;
	 else
	    spr = (RLE_SPRITE *)data[i+j].dat;
      }

      draw_rle_sprite(bmp, spr, x-spr->w/2, y-spr->h/2);

      if (animation_type == DIRTY_RECTANGLE)
	 add_to_list(&dirty, x-spr->h/2, y-spr->h/2, spr->w, spr->h);
   }

   /* draw the bullet */
   if (bullet_flag) {
      x = bullet_x;
      y = bullet_y;

      spr = (RLE_SPRITE *)data[ROCKET].dat;
      draw_rle_sprite(bmp, spr, x-spr->w/2, y-spr->h/2);

      if (animation_type == DIRTY_RECTANGLE)
	 add_to_list(&dirty, x-spr->w/2, y-spr->h/2, spr->w, spr->h);
   }

   /* draw the score and fps information */
   if (fps)
      sprintf(score_buf, "Score: %ld - (%s, %ld fps)", (long)score, 
					     animation_type_str, (long)fps);
   else
      sprintf(score_buf, "Score: %ld", (long)score);

   textout(bmp, font, score_buf, 0, 0, 7);

   if (animation_type == DIRTY_RECTANGLE)
      add_to_list(&dirty, 0, 0, text_length(font, score_buf), 8);

   if (animation_type == DOUBLE_BUFFER) {
      /* when double buffering, just copy the memory bitmap to the screen */
      blit(s, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
   }
   else if ((animation_type == PAGE_FLIP) || (animation_type == RETRACE_FLIP)) {
      /* for page flipping we scroll to display the image */ 
      scroll_screen(0, (current_page==0) ? 0: SCREEN_H);
   }
   else if (animation_type == TRIPLE_BUFFER) {
      /* make sure the last flip request has actually happened */
      do {
      } while (poll_scroll());

      /* post a request to display the page we just drew */
      request_scroll(0, current_page * SCREEN_H);
   }
   else {
      /* for dirty rectangle animation, only copy the areas that changed */
      for (c=0; c<dirty.count; c++)
	 add_to_list(&old_dirty, dirty.rect[c].x, dirty.rect[c].y, 
				 dirty.rect[c].w, dirty.rect[c].h);

      /* sorting the objects really cuts down on bank switching */
      if (!gfx_driver->linear)
	 qsort(old_dirty.rect, old_dirty.count, sizeof(RECT), rect_cmp);

      for (c=0; c<old_dirty.count; c++) {
	 if ((old_dirty.rect[c].w == 1) && (old_dirty.rect[c].h == 1))
	    putpixel(screen, old_dirty.rect[c].x, old_dirty.rect[c].y, 
		     getpixel(bmp, old_dirty.rect[c].x, old_dirty.rect[c].y));
	 else
	    blit(bmp, screen, old_dirty.rect[c].x, old_dirty.rect[c].y, 
			    old_dirty.rect[c].x, old_dirty.rect[c].y, 
			    old_dirty.rect[c].w, old_dirty.rect[c].h);
      }
   }
}



volatile int scroll_count;

void scroll_counter()
{
   scroll_count++;
}

END_OF_FUNCTION(scroll_counter);



void draw_intro_item(int item, int size)
{
   BITMAP *b = (BITMAP *)data[item].dat;
   int w = MIN(SCREEN_W, (SCREEN_W * 2 / size));
   int h = SCREEN_H / size;

   clear(screen);
   stretch_blit(b, screen, 0, 0, b->w, b->h, (SCREEN_W-w)/2, (SCREEN_H-h)/2, w, h);
}



int fade_intro_item(int music_pos, int fade_speed)
{
   while (midi_pos < music_pos) {
      if (keypressed())
	 return TRUE;

      poll_joystick();
      if ((joy[0].button[0].b) || (joy[0].button[1].b))
	 return TRUE;
   }

   set_pallete(data[GAME_PAL].dat);
   fade_out(fade_speed);

   return FALSE;
}



void play_game()
{
   int c;
   BITMAP *b, *b2;
   int esc = FALSE;

   stop_midi();

   /* set up a load of globals */
   dead = shot = FALSE;
   pos = (SCREEN_W/2)<<SPEED_SHIFT;
   xspeed = yspeed = ycounter = 0;
   ship_state = SHIP3;
   ship_burn = FALSE;
   frame_count = fps = 0;
   bullet_flag = FALSE;
   score = 0;
   old_dirty.count = dirty.count = 0;

   skip_count = 0;
   if (SCREEN_W < 400)
      skip_speed = 0;
   else if (SCREEN_W < 700)
      skip_speed = 1;
   else if (SCREEN_W < 1000)
      skip_speed = 2;
   else
      skip_speed = 3;

   for (c=0; c<MAX_STARS; c++) {
      star[c].ox = random() % SCREEN_W;
      star[c].oy = random() % SCREEN_H;
      star[c].z = random() & 7;
   }

   for (c=0; c<MAX_ALIENS; c++) {
      alien[c].x = 16 + random() % (SCREEN_W-32);
      alien[c].y = -60 - (random() & 0x3f);
      alien[c].d = (random()&1) ? 1 : -1;
      alien[c].state = -1;
      alien[c].shot = FALSE;
   }
   alien_count = 2;
   new_alien_count = 0;

   /* introduction synced to the music */
   draw_intro_item(INTRO_BMP_1, 5);
   play_midi(data[GAME_MUSIC].dat, TRUE);
   clear_keybuf();

   if (fade_intro_item(-1, 2))
      goto skip;

   draw_intro_item(INTRO_BMP_2, 4);
   if (fade_intro_item(5, 2))
      goto skip;

   draw_intro_item(INTRO_BMP_3, 3);
   if (fade_intro_item(9, 4))
      goto skip;

   draw_intro_item(INTRO_BMP_4, 2);
   if (fade_intro_item(11, 4))
      goto skip;

   draw_intro_item(GO_BMP, 1);

   if (fade_intro_item(13, 16)) 
      goto skip;

   if (fade_intro_item(14, 16)) 
      goto skip;

   if (fade_intro_item(15, 16)) 
      goto skip;

   if (fade_intro_item(16, 16)) 
      goto skip;

   skip:

   text_mode(0);
   clear(screen);
   clear(s);
   draw_screen();

   if ((!keypressed()) && (!joy[0].button[0].b) && (!joy[0].button[1].b)) {
      do {
      } while (midi_pos < 17);
   }

   clear_keybuf();

   set_pallete(data[GAME_PAL].dat);
   position_mouse(SCREEN_W/2, SCREEN_H/2);

   /* set up the interrupt routines... */
   install_int(fps_proc, 1000);

   if (use_retrace_proc)
      retrace_proc = game_timer;
   else
      install_int(game_timer, 6400/SCREEN_W);

   game_time = 0;

   /* main game loop */
   while (!dead) {
      poll_joystick();

      while (game_time > 0) {
	 move_everyone();
	 game_time--;
      }

      draw_screen();
      frame_count++;

      if (key[KEY_ESC])
	 esc = dead = TRUE;
   }

   /* cleanup, display score, etc */
   remove_int(fps_proc);

   if (use_retrace_proc)
      retrace_proc = NULL;
   else
      remove_int(game_timer);

   stop_sample(data[ENGINE_SPL].dat);

   if (esc) {
      while (current_page != 0)
	 draw_screen();

      do {
      } while (key[KEY_ESC]);

      fade_out(5);
      return;
   }

   fps = 0;
   draw_screen();

   if ((animation_type == PAGE_FLIP) || (animation_type == RETRACE_FLIP) ||
       (animation_type == TRIPLE_BUFFER)) {
      while (current_page != 0)
	 draw_screen();

      blit(screen, s, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
   }

   b = create_bitmap(160, 160);
   b2 = create_bitmap(160, 160);
   clear(b);
   textout_centre(b, data[END_FONT].dat, "GAME OVER", 80, 50, 2);
   textout_centre(b, data[END_FONT].dat, score_buf, 80, 82, 2);
   clear_keybuf();
   scroll_count = -160;
   c = 0;
   install_int(scroll_counter, 6000/SCREEN_W);

   while ((!keypressed()) && (!joy[0].button[0].b) && (!joy[0].button[1].b)) {
      if (scroll_count > c+8)
	 c = scroll_count = c+8;
      else
	 c = scroll_count;

      blit(s, b2, c, c, 0, 0, 160, 160);
      rotate_sprite(b2, b, 0, 0, itofix(c)/4);
      blit(b2, screen, 0, 0, c, c, 160, 160);

      if ((c > SCREEN_W) || (c > SCREEN_H))
	 scroll_count = -160;

      poll_joystick();
   }

   remove_int(scroll_counter);
   destroy_bitmap(b);
   destroy_bitmap(b2);
   clear_keybuf();
   fade_out(5);
}



void generate_explosions()
{
   BITMAP *bmp = create_bitmap(EXPLODE_SIZE, EXPLODE_SIZE);
   unsigned char *p;
   int c, c2;
   int x, y;
   int xx, yy;
   int color;

   #define HOTSPOTS  64

   struct HOTSPOT
   {
      int x, y;
      int xc, yc;
   } hot[HOTSPOTS];

   for (c=0; c<HOTSPOTS; c++) {
      hot[c].x = hot[c].y = (EXPLODE_SIZE/2)<<16;
      hot[c].xc = (random()&0xFFFF)-0x7FFF;
      hot[c].yc = (random()&0xFFFF)-0x7FFF;
   }

   for (c=0; c<EXPLODE_FRAMES; c++) {
      clear(bmp);

      color = ((c<16) ? c*4 : (80-c)) >> 2;

      for (c2=0; c2<HOTSPOTS; c2++) {
	 for (x=-6; x<=6; x++) {
	    for (y=-6; y<=6; y++) {
	       xx = (hot[c2].x>>16) + x;
	       yy = (hot[c2].y>>16) + y;
	       if ((xx>0) && (yy>0) && (xx<EXPLODE_SIZE) && (yy<EXPLODE_SIZE)) {
		  p = bmp->line[yy] + xx;
		  *p += (color >> ((ABS(x)+ABS(y))/3));
		  if (*p > 63)
		     *p = 63;
	       }
	    }
	 }
	 hot[c2].x += hot[c2].xc;
	 hot[c2].y += hot[c2].yc;
      }

      for (x=0; x<EXPLODE_SIZE; x++) {
	 for (y=0; y<EXPLODE_SIZE; y++) {
	    c2 = bmp->line[y][x];
	    if (c2 < 8)
	       bmp->line[y][x] = 0;
	    else
	       bmp->line[y][x] = 16+c2/4;
	 }
      }

      explosion[c] = get_rle_sprite(bmp);
   }

   destroy_bitmap(bmp);
}



int title_screen()
{
   static int color = 0;
   int c, c2;
   fixed x, y;
   int ix, iy;
   int star_count = 0;
   int star_count_count = 0;
   RGB rgb;
   char buf[2] = " ";
   int text_char = 0xFFFF;
   int text_width = 0;
   int text_pix = 0;
   BITMAP *text_bmp;

   play_midi(data[TITLE_MUSIC].dat, TRUE);
   play_sample(data[WELCOME_SPL].dat, 255, 127, 1000, FALSE);

   for (c=0; c<MAX_STARS; c++) {
      star[c].z = 0;
      star[c].ox = star[c].oy = -1;
   }

   for (c=0; c<8; c++)
      title_pallete[c] = ((RGB *)data[TITLE_PAL].dat)[c];

   /* set up the colors differently each time we display the title screen */
   for (c=8; c<PAL_SIZE/2; c++) {
      rgb = ((RGB *)data[TITLE_PAL].dat)[c];
      switch (color) {
	 case 0:
	    rgb.b = rgb.r;
	    rgb.r = 0;
	    break;
	 case 1:
	    rgb.g = rgb.r;
	    rgb.r = 0;
	    break;
	 case 3:
	    rgb.g = rgb.r;
	    break;
      }
      title_pallete[c] = rgb;
   }

   for (c=PAL_SIZE/2; c<PAL_SIZE; c++)
      title_pallete[c] = ((RGB *)data[TITLE_PAL].dat)[c];

   color++;
   if (color > 3)
      color = 0;

   text_bmp = create_bitmap(SCREEN_W, 24);
   clear(text_bmp);

   clear(screen);
   set_pallete(title_pallete);

   scroll_count = 1;
   install_int(scroll_counter, 5);

   while ((c=scroll_count) < 160)
      stretch_blit(data[TITLE_BMP].dat, screen, 0, 0, 320, 128,
		   SCREEN_W/2-c, SCREEN_H/2-c*64/160-32, c*2, c*128/160);

   remove_int(scroll_counter);
   blit(data[TITLE_BMP].dat, screen, 0, 0, SCREEN_W/2-160, SCREEN_H/2-96, 320, 128);

   clear_keybuf();

   scroll_count = 0;

   if (use_retrace_proc)
      retrace_proc = scroll_counter;
   else
      install_int(scroll_counter, 6);

   do {
      /* animate the starfield */
      for (c=0; c<MAX_STARS; c++) {
	 if (star[c].z <= itofix(1)) {
	    x = itofix(random()&0xff);
	    y = itofix(((random()&3)+1)*SCREEN_W);
	    star[c].x = fmul(fcos(x), y);
	    star[c].y = fmul(fsin(x), y);
	    star[c].z = itofix((random() & 0x1f) + 0x20);
	 }

	 x = fdiv(star[c].x, star[c].z);
	 y = fdiv(star[c].y, star[c].z);
	 ix = (int)(x>>16) + SCREEN_W/2;
	 iy = (int)(y>>16) + SCREEN_H/2;
	 putpixel(screen, star[c].ox, star[c].oy, 0);
	 if ((ix >= 0) && (ix < SCREEN_W) && (iy >= 0) && (iy <= SCREEN_H)) {
	    if (getpixel(screen, ix, iy) == 0) {
	       if (c < star_count) {
		  c2 = 7-(int)(star[c].z>>18);
		  putpixel(screen, ix, iy, MID(0, c2, 7));
	       }
	       star[c].ox = ix;
	       star[c].oy = iy;
	    }
	    star[c].z -= 4096;
	 }
	 else
	    star[c].z = 0;
      }

      if (star_count < MAX_STARS) {
	 if (star_count_count++ >= 32) {
	    star_count_count = 0;
	    star_count++;
	 }
      }

      /* wait a bit if we need to */
      do {
      } while (scroll_count <= 0);

      /* and move the text scroller */
      c = use_retrace_proc ? scroll_count*2 : scroll_count;
      scroll_count = 0;
      blit(text_bmp, text_bmp, c, 0, 0, 0, SCREEN_W, 24);
      rectfill(text_bmp, SCREEN_W-c, 0, SCREEN_W, 24, 0);

      while (c > 0) {
	 text_pix += c;
	 textout(text_bmp, data[TITLE_FONT].dat, buf, SCREEN_W-text_pix, 0, -1);
	 if (text_pix >= text_width) {
	    c = text_pix - text_width;
	    text_char++;
	    if (text_char >= data[TITLE_TEXT].size)
	       text_char = 0;
	    buf[0] = ((char *)data[TITLE_TEXT].dat)[text_char];
	    text_pix = 0;
	    text_width = text_length(data[TITLE_FONT].dat, buf);
	 }
	 else
	    c = 0;
      }
      blit(text_bmp, screen, 0, 0, 0, SCREEN_H-24, SCREEN_W, 24);

      poll_joystick();

   } while ((!keypressed()) && (!joy[0].button[0].b) && (!joy[0].button[1].b));

   if (use_retrace_proc)
      retrace_proc = NULL;
   else
      remove_int(scroll_counter);

   fade_out(5);

   while (keypressed())
      if ((readkey() & 0xff) == 27)
	 return FALSE;

   destroy_bitmap(text_bmp);

   return TRUE;
}



void set_gui_colors()
{
   static RGB black = { 0,  0,  0,  0 };
   static RGB grey  = { 48, 48, 48, 0 };
   static RGB white = { 63, 63, 63, 0 };

   set_color(0,   &black);
   set_color(16,  &black);
   set_color(1,   &grey); 
   set_color(255, &white); 

   gui_fg_color = 0;
   gui_bg_color = 1;
}



char *anim_list_getter(int index, int *list_size)
{
   static char *s[] =
   {
      "Double buffered",
      "Page flipping",
      "Synced flips",
      "Triple buffered",
      "Dirty rectangles"
   };

   if (index < 0) {
      *list_size = 5;
      return NULL;
   }

   return s[index];
}



extern DIALOG anim_type_dlg[];


int anim_list_proc(int msg, DIALOG *d, int c)
{
   int sel, ret;

   sel = d->d1;

   ret = d_list_proc(msg, d, c);

   if (sel != d->d1)
      ret |= D_REDRAW;

   return ret;
}



int anim_desc_proc(int msg, DIALOG *d, int c)
{
   static char *double_buffer_desc[] = 
   {
      "Draws onto a memory bitmap,",
      "and then uses a brute-force",
      "blit to copy the entire",
      "image across to the screen.",
      NULL
   };

   static char *page_flip_desc[] = 
   {
      "Uses two pages of video",
      "memory, and flips back and",
      "forth between them. It will",
      "only work if there is enough",
      "video memory to set up dual",
      "pages.",
      NULL
   };

   static char *retrace_flip_desc[] = 
   {
      "This is basically the same",
      "as page flipping, but it uses",
      "the vertical retrace interrupt",
      "simulator instead of retrace",
      "polling. Only works in mode-X,",
      "and not under win95.",
      NULL
   };

   static char *triple_buffer_desc[] = 
   {
      "Uses three pages of video",
      "memory, to avoid wasting time",
      "waiting for retraces. Only",
      "works in mode-X, or with a",
      "VBE 3.0 driver and suitable",
      "SVGA hardware.",
      NULL
   };

   static char *dirty_rectangle_desc[] = 
   {
      "This is similar to double",
      "buffering, but stores a list",
      "of which parts of the screen",
      "have changed, to minimise the",
      "amount of drawing that needs",
      "to be done.",
      NULL
   };

   static char **descs[] =
   {
      double_buffer_desc,
      page_flip_desc,
      retrace_flip_desc,
      triple_buffer_desc,
      dirty_rectangle_desc
   };

   char **p;
   int y;

   if (msg == MSG_DRAW) {
      rectfill(screen, d->x, d->y, d->x+d->w, d->y+d->h, d->bg);
      text_mode(d->bg);

      p = descs[anim_type_dlg[2].d1];
      y = d->y;

      while (*p) {
	 textout(screen, font, *p, d->x, y, d->fg);
	 y += 8;
	 p++;
      } 
   }

   return D_O_K;
}



DIALOG anim_type_dlg[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)  (d2)  (dp)                 (dp2) (dp3) */
   { d_shadow_box_proc, 0,    0,    280,  150,  0,    1,    0,    0,          0,    0,    NULL,                NULL, NULL  },
   { d_ctext_proc,      140,  8,    1,    1,    0,    1,    0,    0,          0,    0,    "Animation Method",  NULL, NULL  },
   { anim_list_proc,    16,   28,   152,  43,   0,    1,    0,    D_EXIT,     0,    0,    anim_list_getter,    NULL, NULL  },
   { anim_desc_proc,    16,   90,   248,  48,   0,    1,    0,    0,          0,    0,    0,                   NULL, NULL  },
   { d_button_proc,     184,  28,   80,   16,   0,    1,    13,   D_EXIT,     0,    0,    "OK",                NULL, NULL  },
   { d_button_proc,     184,  50,   80,   16,   0,    1,    27,   D_EXIT,     0,    0,    "Cancel",            NULL, NULL  },
   { NULL,              0,    0,    0,    0,    0,    0,    0,    0,          0,    0,    NULL,                NULL, NULL  }
};



int pick_animation_type(int *type, int card, int w, int h)
{
   int ret;

   centre_dialog(anim_type_dlg);

   if (((card == GFX_MODEX) || ((w < 640) && ((w != 320) || (h != 200)))) && 
       (w*h*2 <= 256*1024)) {
      if (windows_version != 0)
	 anim_type_dlg[2].d1 = 1;
      else if (w*h*3 <= 256*1024)
	 anim_type_dlg[2].d1 = 3;
      else
	 anim_type_dlg[2].d1 = 2;
   }
   else if (card == GFX_VBEAF)
      anim_type_dlg[2].d1 = 1;
   else
      anim_type_dlg[2].d1 = 4;

   clear(screen);

   ret = do_dialog(anim_type_dlg, 2);

   *type = anim_type_dlg[2].d1 + 1;

   return (ret == 5) ? -1 : ret;
}



int main(int argc, char *argv[])
{
   int c, w, h, vh;
   char buf[256];

   for (c=1; c<argc; c++) {
      if (stricmp(argv[c], "-cheat") == 0)
	 cheat = TRUE;

      if (stricmp(argv[c], "-turbo") == 0)
	 turbo = TRUE;
   }

   allegro_init();
   install_keyboard();
   install_mouse();
   install_timer();

   if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, argv[0]) != 0) {
      printf("\nError initialising sound\n%s\n\n", allegro_error);
      exit(1);
   }

   if (install_joystick(JOY_TYPE_AUTODETECT) != 0) {
      printf("\nError initialising joystick\n%s\n\n", allegro_error);
      exit(1);
   }

   fade_out(4);
   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);

   replace_filename(buf, argv[0], "demo.dat", sizeof(buf));
   data = load_datafile(buf);
   if (!data) {
      allegro_exit();
      printf("Error loading %s\n\n", buf);
      exit(1);
   }

   if ((midi_driver->id == MIDI_AWE32) || (midi_driver->id == MIDI_DIGMID))
      play_midi(data[INTRO_MUSIC].dat, FALSE);
   else 
      play_sample(data[INTRO_SPL].dat, 255, 128, 1000, FALSE);

   play_memory_fli(data[INTRO_ANIM].dat, screen, FALSE, NULL);

   c = retrace_count;

   generate_explosions();

   do {
   } while (retrace_count-c < 120);

   fade_out(1);

   clear(screen);
   set_gui_colors();

   if (!gfx_mode_select(&c, &w, &h)) {
      allegro_exit();
      exit(1);
   }

   if (pick_animation_type(&animation_type, c, w, h) < 0) {
      allegro_exit();
      exit(1);
   }

   if (animation_type == PAGE_FLIP) {
      vh = h * 2;
   }
   else if (animation_type == RETRACE_FLIP) {
      vh = h*2;

      if (c == GFX_AUTODETECT)
	 c = GFX_MODEX;

      if (c != GFX_MODEX) {
	 allegro_exit();
	 printf("Error: retrace simulation is only possible in mode-X\n\n");
	 exit(1);
      }
   }
   else  if (animation_type == TRIPLE_BUFFER) {
      vh = h*3;
   }
   else
      vh = 0;

   if (set_gfx_mode(c, w, h, 0, vh) != 0) {
      allegro_exit();
      printf("Error setting graphics mode\n%s\n\n", (w < 640) ? "Try an animation type that requires fewer pages of video memory" : allegro_error);
      exit(1);
   }

   if ((animation_type == PAGE_FLIP) || (animation_type == RETRACE_FLIP) ||
       (animation_type == TRIPLE_BUFFER)) {
      page1 = create_sub_bitmap(screen, 0, 0, SCREEN_W, SCREEN_H);
      page2 = create_sub_bitmap(screen, 0, SCREEN_H, SCREEN_W, SCREEN_H);

      if (animation_type == TRIPLE_BUFFER) {
	 if (!(gfx_capabilities & GFX_CAN_TRIPLE_BUFFER)) {
	    strcpy(buf, gfx_driver->name);
	    allegro_exit();
	    printf("The %s driver does not support triple buffering\n\n", buf);
	    exit(1);
	 }

	 if (gfx_driver->id == GFX_MODEX) {
	    if (windows_version != 0) {
	       set_gui_colors();
	       alert("Warning: mode-X triple buffering", "uses the timer retrace simulator,", "which only works well from DOS", "Ok", NULL, 13, 0);
	    }

	    timer_simulate_retrace(TRUE);
	    use_retrace_proc = TRUE;
	 }

	 page3 = create_sub_bitmap(screen, 0, SCREEN_H*2, SCREEN_W, SCREEN_H);
      }
      else if (gfx_driver->id == GFX_MODEX) {
	 use_retrace_proc = TRUE;

	 if (animation_type == RETRACE_FLIP)
	    timer_simulate_retrace(TRUE);
      }
   }

   LOCK_VARIABLE(game_time);
   LOCK_FUNCTION(game_timer);

   LOCK_VARIABLE(scroll_count);
   LOCK_FUNCTION(scroll_counter);

   LOCK_VARIABLE(score);
   LOCK_VARIABLE(frame_count);
   LOCK_VARIABLE(fps);
   LOCK_FUNCTION(fps_proc);

   text_mode(0);

   s = create_bitmap(SCREEN_W, SCREEN_H);

   stop_sample(data[INTRO_SPL].dat);

   while (title_screen())
      play_game(); 

   allegro_exit();

#ifdef DJGPP
   cputs(data[END_TEXT].dat);
#else
   printf("%s", data[END_TEXT].dat);
#endif

   destroy_bitmap(s);

   if ((animation_type == PAGE_FLIP) || (animation_type == RETRACE_FLIP) ||
       (animation_type == TRIPLE_BUFFER)) {
      destroy_bitmap(page1);
      destroy_bitmap(page2);

      if (animation_type == TRIPLE_BUFFER)
	 destroy_bitmap(page3);
   }

   for (c=0; c<EXPLODE_FRAMES; c++)
      destroy_rle_sprite(explosion[c]);

   unload_datafile(data);
   return 0;
}

