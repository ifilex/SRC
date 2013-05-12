/*
 *    3Demo - 3D Graphics Demonstrator by Dave Thomson.
 *
 *    This program draws a 3D starfield (depth-cued) and
 *    a polygon starship (controllable).  Uses the Allegro
 *    maths functions to do it.
 */

#include <stdio.h>
#include "allegro.h"

typedef struct VECTOR
{
  fixed x, y, z;
} VECTOR;

/******************************************** STARFIELD SYSTEM *****/

#define NUM_STARS         256         /* number of stars in starfield */

#define Z_NEAR            24
#define Z_FAR             1024
#define XY_CUBE           512

#define SPEED_LIMIT       20

VECTOR stars[NUM_STARS];
fixed star_x[NUM_STARS], star_y[NUM_STARS];
VECTOR delta;                    /* the same for the whole field */

void init_stars (void);
void draw_stars (void);
void erase_stars (void);
void move_stars (void);

/********************************************** POLYGON MODELS *****/

#define NUM_VERTS         4
#define NUM_FACES         4

#define ENGINE            3           /* which face is the engine */
#define ENGINE_ON         64          /* colour index */
#define ENGINE_OFF        32

typedef struct FACE                /* for triangular models */
{
  int v1, v2, v3;
  int colour, range;
  VECTOR normal, rnormal;
} FACE;

typedef struct MODEL
{
  VECTOR points[NUM_VERTS];
  FACE faces[NUM_FACES];
  fixed x, y, z;
  fixed rx, ry, rz;
  int minx, miny, maxx, maxy;
  VECTOR aim;
  int active, velocity;
} MODEL;

MODEL ship;
VECTOR direction;

void init_ship (void);
void draw_ship (void);
void erase_ship (void);

/********************************************* PARTICLE SYSTEM *****/

#define NUM_PARTS     100
#define COUNTDOWN     30

typedef struct PARTICLE
{
  int x, y;
  int xvel, yvel;
  int colour, active, counter, behind;
} PARTICLE;

PARTICLE explode[NUM_PARTS];

void init_explode (void);
void draw_explode (void);
void erase_explode (void);

/******************************************* MAIN PROGRAM CODE *****/

BITMAP *buffer;

int main (int argc, char **argv)
{
  PALETTE pal;
  char buf[64];
  int i, wait = -1;

  allegro_init ();
  install_keyboard ();
  install_timer ();

  if (argc > 1)
  {
    if (set_gfx_mode (GFX_VGA, 320, 200, 0, 0) != 0)
    {
      printf ("error setting VGA 320x200x256\n");
      return 1;
    }
  }
  else 
  {
    if (set_gfx_mode (GFX_AUTODETECT, 640, 480, 0, 0) != 0)
    {
      printf ("error setting SVGA 640x480x256\nTry giving a parameter to set 320x200x256 mode\n");
      return 1;
    }
  }

  for (i=0; i<16; i++)
    pal[i].r = pal[i].g = pal[i].b = 0;

  /* greyscale */
  pal[16].r = pal[16].g = pal[16].b = 63;
  pal[17].r = pal[17].g = pal[17].b = 48;
  pal[18].r = pal[18].g = pal[18].b = 32;
  pal[19].r = pal[19].g = pal[19].b = 16;
  pal[20].r = pal[20].g = pal[20].b = 8;

  for (i=0; i<16; i++)   /* red range */
  {
    pal[i+32].r = 31+(i*2);
    pal[i+32].g = 15;
    pal[i+32].b = 7;
  }
  for (i=64; i<68; i++)
  {
    pal[i].r = 63;      /* a nice fire orange */
    pal[i].g = 17+((i-64)*3);
    pal[i].b = 0;
  }

  set_palette (pal);

  buffer = create_bitmap (SCREEN_W, SCREEN_H);
  clear (buffer);

  set_projection_viewport (0, 0, SCREEN_W, SCREEN_H);

  init_stars ();
  draw_stars ();
  init_ship ();
  draw_ship ();

  for (;;)
  {
    erase_stars ();
    if (ship.active)
      erase_ship ();
    else
      erase_explode ();

    move_stars ();
    draw_stars ();

    sprintf (buf, "     direction: [%f] [%f] [%f]     ", fixtof(direction.x), fixtof(direction.y), fixtof(direction.z));
    textout_centre (buffer, font, buf, SCREEN_W / 2, SCREEN_H - 10, 17);
    sprintf (buf, "   delta: [%f] [%f] [%f]   ", fixtof(delta.x), fixtof(delta.y), fixtof(delta.z));
    textout_centre (buffer, font, buf, SCREEN_W / 2, SCREEN_H - 20, 17);
    sprintf (buf, "   velocity: %d   ", ship.velocity);
    textout_centre (buffer, font, buf, SCREEN_W / 2, SCREEN_H - 30, 17);

    textout_centre (buffer, font, "3Demo by Dave Thomson", SCREEN_W / 2, 0, 16);
    textout_centre (buffer, font, "gameskitchen@geocities.com", SCREEN_W / 2, 8, 17);
    textout_centre (buffer, font, "Press ESC to exit", SCREEN_W / 2, 16, 18);
    textout_centre (buffer, font, "Press SPACE to explode", SCREEN_W / 2, 24, 18);
    textout_centre (buffer, font, "Press CTRL to fire engine", SCREEN_W / 2, 32, 18);

    if (ship.active)
      draw_ship ();
    else
      draw_explode ();

    vsync ();
    blit (buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

    if (wait > 0 && !(ship.active))
    {
      wait--;
      if (ship.velocity > 0)
	ship.velocity--;
      if (ship.velocity < 0)
	ship.velocity = 0;
    }
    else if (wait == 0)
    {
      ship.active = TRUE;
      wait = -1;
    }

    if (key[KEY_ESC])                         /* exit program */
      break;

    if (key[KEY_SPACE] && ship.active)        /* activate explosion */
    {
      init_explode ();
      wait = COUNTDOWN << 1;
    }

    if (ship.active)
    { 
      if (key[KEY_UP])                                      /* rotates */
	ship.rx -= itofix(5);
      else if (key[KEY_DOWN])
	ship.rx += itofix(5);
      if (key[KEY_LEFT])
	ship.ry -= itofix(5);
      else if (key[KEY_RIGHT])
	ship.ry += itofix(5);
      if (key[KEY_PGUP])
	ship.rz -= itofix(5);
      else if (key[KEY_PGDN])
	ship.rz += itofix(5);

      if (key[KEY_CONTROL])                                 /* thrust */
      {
	ship.faces[ENGINE].colour = ENGINE_ON;
	ship.faces[ENGINE].range = 3;
	if (ship.velocity < SPEED_LIMIT)
	  ship.velocity += 2;
      }
      else
      {
	ship.faces[ENGINE].colour = ENGINE_OFF;
	ship.faces[ENGINE].range = 15;
	if (ship.velocity > 0)
	  ship.velocity -= 2;
      }

      ship.rx = ship.rx & itofix(255);
      ship.ry = ship.ry & itofix(255);
      ship.rz = ship.rz & itofix(255);
    }   /* if (ship.active) */

    delta.x = fmul (direction.x, itofix(ship.velocity));
    delta.y = fmul (direction.y, itofix(ship.velocity));
    delta.z = fmul (direction.z, itofix(ship.velocity));
  }

  destroy_bitmap (buffer);
  return 0;
}

void init_stars (void)
{
  int i;

  for (i=0; i<NUM_STARS; i++)
  {
    stars[i].x = itofix ((random() % XY_CUBE) - (XY_CUBE >> 1));
    stars[i].y = itofix ((random() % XY_CUBE) - (XY_CUBE >> 1));
    stars[i].z = itofix ((random() % (Z_FAR - Z_NEAR)) + Z_NEAR);
  }
  delta.x = itofix(0);
  delta.y = itofix(0);
  delta.z = itofix(0);
}

void draw_stars (void)
{
  int i, c;
  MATRIX m;
  VECTOR outs[NUM_STARS];

  for (i=0; i<NUM_STARS; i++)
  {
    get_translation_matrix (&m, delta.x, delta.y, delta.z);
    apply_matrix (&m, stars[i].x, stars[i].y, stars[i].z,
			  &outs[i].x, &outs[i].y, &outs[i].z);
    persp_project (outs[i].x, outs[i].y, outs[i].z, &star_x[i], &star_y[i]);
    c = (fixtoi(outs[i].z) >> 8) + 16;
    putpixel (buffer, fixtoi(star_x[i]), fixtoi(star_y[i]), c);
  }
}

void erase_stars (void)
{
  int i;

  for (i=0; i<NUM_STARS; i++)
    putpixel (buffer, fixtoi(star_x[i]), fixtoi(star_y[i]), 0);
}

void move_stars (void)
{
  int i;

  for (i=0; i<NUM_STARS; i++)
  {
    stars[i].x += delta.x;
    stars[i].y += delta.y;
    stars[i].z += delta.z;

    if (stars[i].x > itofix(XY_CUBE >> 1))
      stars[i].x = itofix(-(XY_CUBE >> 1));
    else if (stars[i].x < itofix(-(XY_CUBE >> 1)))
      stars[i].x = itofix(XY_CUBE >> 1);

    if (stars[i].y > itofix(XY_CUBE >> 1))
      stars[i].y = itofix(-(XY_CUBE >> 1));
    else if (stars[i].y < itofix(-(XY_CUBE >> 1)))
      stars[i].y = itofix(XY_CUBE >> 1);

    if (stars[i].z > itofix(Z_FAR))
      stars[i].z = itofix(Z_NEAR);
    else if (stars[i].z < itofix(Z_NEAR))
      stars[i].z = itofix(Z_FAR);
  }
}

void init_ship (void)
{
  int i;
  FACE *face;
  VECTOR v1, v2, *pts;

  ship.points[0].x = itofix (  0);
  ship.points[0].y = itofix (  0);
  ship.points[0].z = itofix ( 32);

  ship.points[1].x = itofix ( 16);
  ship.points[1].y = itofix (-16);
  ship.points[1].z = itofix (-32);

  ship.points[2].x = itofix (-16);
  ship.points[2].y = itofix (-16);
  ship.points[2].z = itofix (-32);

  ship.points[3].x = itofix (  0);
  ship.points[3].y = itofix ( 16);
  ship.points[3].z = itofix (-32);

  ship.faces[0].v1 = 3;
  ship.faces[0].v2 = 0;
  ship.faces[0].v3 = 1;
  pts = &ship.points[0];
  face = &ship.faces[0];
  v1.x = (pts[face->v2].x - pts[face->v1].x);
  v1.y = (pts[face->v2].y - pts[face->v1].y);
  v1.z = (pts[face->v2].z - pts[face->v1].z);
  v2.x = (pts[face->v3].x - pts[face->v1].x);
  v2.y = (pts[face->v3].y - pts[face->v1].y);
  v2.z = (pts[face->v3].z - pts[face->v1].z);
  cross_product (v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, 
		&(face->normal.x), &(face->normal.y), &(face->normal.z));

  ship.faces[1].v1 = 2;
  ship.faces[1].v2 = 0;
  ship.faces[1].v3 = 3;
  face = &ship.faces[1];
  v1.x = (pts[face->v2].x - pts[face->v1].x);
  v1.y = (pts[face->v2].y - pts[face->v1].y);
  v1.z = (pts[face->v2].z - pts[face->v1].z);
  v2.x = (pts[face->v3].x - pts[face->v1].x);
  v2.y = (pts[face->v3].y - pts[face->v1].y);
  v2.z = (pts[face->v3].z - pts[face->v1].z);
  cross_product (v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, 
		&(face->normal.x), &(face->normal.y), &(face->normal.z));

  ship.faces[2].v1 = 1;
  ship.faces[2].v2 = 0;
  ship.faces[2].v3 = 2;
  face = &ship.faces[2];
  v1.x = (pts[face->v2].x - pts[face->v1].x);
  v1.y = (pts[face->v2].y - pts[face->v1].y);
  v1.z = (pts[face->v2].z - pts[face->v1].z);
  v2.x = (pts[face->v3].x - pts[face->v1].x);
  v2.y = (pts[face->v3].y - pts[face->v1].y);
  v2.z = (pts[face->v3].z - pts[face->v1].z);
  cross_product (v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, 
		&(face->normal.x), &(face->normal.y), &(face->normal.z));

  ship.faces[3].v1 = 2;
  ship.faces[3].v2 = 3;
  ship.faces[3].v3 = 1;
  face = &ship.faces[3];
  v1.x = (pts[face->v2].x - pts[face->v1].x);
  v1.y = (pts[face->v2].y - pts[face->v1].y);
  v1.z = (pts[face->v2].z - pts[face->v1].z);
  v2.x = (pts[face->v3].x - pts[face->v1].x);
  v2.y = (pts[face->v3].y - pts[face->v1].y);
  v2.z = (pts[face->v3].z - pts[face->v1].z);
  cross_product (v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, 
		&(face->normal.x), &(face->normal.y), &(face->normal.z));

  for (i=0; i<NUM_FACES; i++)
  {
    ship.faces[i].colour = 32;
    ship.faces[i].range = 15;
    normalize_vector (&ship.faces[i].normal.x, &ship.faces[i].normal.y, &ship.faces[i].normal.z);
    ship.faces[i].rnormal.x = ship.faces[i].normal.x;
    ship.faces[i].rnormal.y = ship.faces[i].normal.y;
    ship.faces[i].rnormal.z = ship.faces[i].normal.z;
  }

  ship.x = ship.y = 0;
  ship.z = itofix (192);
  ship.rx = ship.ry = ship.rz = 0;

  ship.aim.x = direction.x = 0;
  ship.aim.y = direction.y = 0;
  ship.aim.z = direction.z = itofix(-1);
  ship.velocity = 0;
  ship.active = TRUE;
}

void draw_ship (void)
{
  int i, col;
  MATRIX m;
  VECTOR outs[NUM_VERTS];

  ship.minx = SCREEN_W; 
  ship.miny = SCREEN_H;
  ship.maxx = ship.maxy = 0;

  get_rotation_matrix (&m, ship.rx, ship.ry, ship.rz);
  apply_matrix (&m, ship.aim.x, ship.aim.y, ship.aim.z,
		  &outs[0].x, &outs[0].y, &outs[0].z);
  direction.x = outs[0].x;
  direction.y = outs[0].y;
  direction.z = outs[0].z;

  for (i=0; i<NUM_FACES; i++)
    apply_matrix (&m, ship.faces[i].normal.x, ship.faces[i].normal.y, ship.faces[i].normal.z,
		    &ship.faces[i].rnormal.x, &ship.faces[i].rnormal.y, &ship.faces[i].rnormal.z);

  get_transformation_matrix (&m, itofix(1), ship.rx, ship.ry, ship.rz, 
				ship.x, ship.y, ship.z);
  for (i=0; i<NUM_VERTS; i++)
  {
    apply_matrix (&m, ship.points[i].x, ship.points[i].y, ship.points[i].z,
		      &outs[i].x, &outs[i].y, &outs[i].z);
    persp_project (outs[i].x, outs[i].y, outs[i].z, &outs[i].x, &outs[i].y);
    if (fixtoi(outs[i].x) < ship.minx)
      ship.minx = fixtoi(outs[i].x);
    if (fixtoi(outs[i].x) > ship.maxx)
      ship.maxx = fixtoi(outs[i].x);
    if (fixtoi(outs[i].y) < ship.miny)
      ship.miny = fixtoi(outs[i].y);
    if (fixtoi(outs[i].y) > ship.maxy)
      ship.maxy = fixtoi(outs[i].y);
  }

  for (i=0; i<NUM_FACES; i++)
  {
    if (fixtof(ship.faces[i].rnormal.z) < 0.0)
    {
      col = fixtoi(fmul (dot_product (ship.faces[i].rnormal.x, ship.faces[i].rnormal.y, ship.faces[i].rnormal.z, 0, 0, itofix(1)),
		  itofix(ship.faces[i].range)));
      if (col < 0)
	col = -col + ship.faces[i].colour;
      else
	col = col + ship.faces[i].colour; 
      triangle (buffer, fixtoi(outs[ship.faces[i].v1].x), fixtoi(outs[ship.faces[i].v1].y),
			fixtoi(outs[ship.faces[i].v2].x), fixtoi(outs[ship.faces[i].v2].y),
			fixtoi(outs[ship.faces[i].v3].x), fixtoi(outs[ship.faces[i].v3].y),
			col);
    }
  }
}

void erase_ship (void)
{
  rectfill (buffer, ship.minx, ship.miny, ship.maxx, ship.maxy, 0);
}

void init_explode (void)
{
  int i;

  for (i=0; i<NUM_PARTS; i++)
  {
    explode[i].x = (SCREEN_W >> 1) + (random() % 30) -15;
    explode[i].y = (SCREEN_H >> 1) + (random() % 30) - 15;
    explode[i].colour = (random() % 4) + 64;
    explode[i].active = TRUE;
    explode[i].counter = ((random() % 10) - 5) + COUNTDOWN;
    explode[i].xvel = random() % 4 - 2;
    explode[i].yvel = random() % 4 - 2;
  }
  ship.active = FALSE;
  erase_ship ();
}

void draw_explode (void)
{
  int i;

  for (i=0; i<NUM_PARTS; i++)
  {
    if (explode[i].active)
    {
      explode[i].x += explode[i].xvel;
      explode[i].y += explode[i].yvel;
      explode[i].behind = getpixel (buffer, explode[i].x, explode[i].y);
      putpixel (buffer, explode[i].x, explode[i].y, explode[i].colour);
    }
    if (explode[i].counter > -1)
      explode[i].counter--;
    if (explode[i].counter == 0)
      explode[i].active = FALSE;
  }
}

void erase_explode (void)
{
  int i;

  for (i=NUM_PARTS-1; i>=0; i--)
  {
    if (explode[i].counter != -1)
      putpixel (buffer, explode[i].x, explode[i].y, explode[i].behind);
  }
}

