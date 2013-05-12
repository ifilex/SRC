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
 *      Grabber plugin for managing bitmapped image objects. Regular 
 *      bitmaps, RLE sprites, and compiled sprites are all handled in 
 *      this one file, because they can share a lot of common code.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "allegro.h"
#include "../datedit.h"



/* creates a new bitmap object */
static void *makenew_bitmap(long *size)
{
   BITMAP *bmp = create_bitmap_ex(8, 32, 32);

   clear(bmp);
   text_mode(-1);
   textout_centre(bmp, font, "Hi!", 16, 12, 1);

   return bmp;
}



/* returns a bitmap description string */
static void get_bitmap_desc(DATAFILE *dat, char *s)
{
   BITMAP *bmp = (BITMAP *)dat->dat;

   sprintf(s, "bitmap (%dx%d, %d bit)", bmp->w, bmp->h, bitmap_color_depth(bmp));
}



/* exports a bitmap into an external file */
static int export_bitmap(DATAFILE *dat, char *filename)
{
   return (save_bitmap(filename, (BITMAP *)dat->dat, datedit_current_palette) == 0);
}



/* draws a bitmap onto the grabber object view window */
static void plot_bitmap(DATAFILE *dat, int x, int y)
{
   BITMAP *b = dat->dat;
   int w = b->w;
   int h = b->h;
   fixed scale;

   if (w > SCREEN_W-x-8) {
      scale = itofix(SCREEN_W-x-8) / w;
      w = (w * scale) >> 16;
      h = (h * scale) >> 16;
   }

   if (h > SCREEN_H-y-40) {
      scale = itofix(SCREEN_H-y-40) / h;
      w = (w * scale) >> 16;
      h = (h * scale) >> 16;
   }

   rect(screen, x, y+32, x+w+1, y+32+h+1, gui_fg_color);

   if ((bitmap_color_depth(screen) == 8) && (bitmap_color_depth(b) > 8)) {
      if ((w != b->w) || (h != b->h))
	 textout(screen, font, "<reduced color scaled preview>", x, y+18, gui_fg_color);
      else
	 textout(screen, font, "<reduced color preview>", x, y+18, gui_fg_color);
   }
   else if ((w != b->w) || (h != b->h))
      textout(screen, font, "<scaled preview>", x, y+18, gui_fg_color);

   if (bitmap_color_depth(b) == bitmap_color_depth(screen)) {
      if (dat->type != DAT_BITMAP)
	 stretch_sprite(screen, b, x+1, y+33, w, h);
      else
	 stretch_blit(b, screen, 0, 0, b->w, b->h, x+1, y+33, w, h);
   }
   else {
      BITMAP *b2 = create_bitmap_ex(bitmap_color_depth(b), w, h);
      clear_to_color(b2, makecol_depth(bitmap_color_depth(b2), 0x80, 0x80, 0x80));

      if (dat->type != DAT_BITMAP) {
	 RGB tmprgb = datedit_current_palette[0];

	 datedit_current_palette[0].r = 63;
	 datedit_current_palette[0].g = 0;
	 datedit_current_palette[0].b = 63;
	 select_palette(datedit_current_palette);

	 stretch_sprite(b2, b, 0, 0, w, h);
	 blit(b2, screen, 0, 0, x+1, y+33, w, h);

	 unselect_palette();
	 datedit_current_palette[0] = tmprgb;
      }
      else {
	 stretch_blit(b, b2, 0, 0, b->w, b->h, 0, 0, w, h);
	 blit(b2, screen, 0, 0, x+1, y+33, w, h);
      }

      destroy_bitmap(b2);
   }
}



/* handles double-clicking on a bitmap in the grabber */
static int view_bitmap(DATAFILE *dat)
{
   BITMAP *b = dat->dat;
   fixed scale = itofix(1);
   fixed prevscale = itofix(1);
   BITMAP *bc = NULL;
   int done = FALSE;
   int c;

   show_mouse(NULL);
   clear_to_color(screen, gui_mg_color);
   blit(b, screen, 0, 0, 0, 0, b->w, b->h);

   clear_keybuf();
   do {
   } while (mouse_b);

   do {
      if (scale != prevscale) {
	 clear_to_color(screen, gui_mg_color);

	 if (!bc) {
	    bc = create_bitmap(b->w, b->h);
	    blit(b, bc, 0, 0, 0, 0, b->w, b->h);
	 }

	 stretch_blit(bc, screen, 0, 0, b->w, b->h, 0, 0, (b->w*scale)>>16, (b->h*scale)>>16);
	 prevscale = scale;
      }

      if (keypressed()) {
	 c = readkey();

	 switch (c >> 8) {

	    case KEY_DOWN:
	    case KEY_RIGHT:
	       if (scale < itofix(16))
		  scale *= 2;
	       break;

	    case KEY_UP:
	    case KEY_LEFT:
	       if (scale > itofix(1)/16)
		  scale /= 2;
	       break;

	    default:
	       switch (c & 0xFF) {

		  case '+':
		     if (scale < itofix(16))
			scale *= 2;
		     break;

		  case '-':
		     if (scale > itofix(1)/16)
			scale /= 2;
		     break;

		  default:
		     done = TRUE;
		     break;
	       }
	       break;
	 }
      }

      if (mouse_b)
	 done = TRUE;

   } while (!done);

   if (bc)
      destroy_bitmap(bc);

   clear_keybuf();
   do {
   } while (mouse_b);

   clear(screen);
   show_mouse(screen);

   return D_REDRAW;
}



/* reads a bitmap from an external file */
static void *grab_bitmap(char *filename, long *size, int x, int y, int w, int h, int depth)
{
   BITMAP *bmp;

   if (depth > 0) {
      int oldcolordepth = _color_depth;

      _color_depth = depth;
      set_color_conversion(COLORCONV_TOTAL);

      bmp = load_bitmap(filename, datedit_last_read_pal);

      _color_depth = oldcolordepth;
      set_color_conversion(COLORCONV_NONE);
   }
   else
      bmp = load_bitmap(filename, datedit_last_read_pal);

   if (!bmp)
      return NULL;

   if ((x >= 0) && (y >= 0) && (w >= 0) && (h >= 0)) {
      BITMAP *b2 = create_bitmap_ex(bitmap_color_depth(bmp), w, h);
      clear_to_color(b2, bitmap_mask_color(b2));
      blit(bmp, b2, x, y, 0, 0, w, h);
      destroy_bitmap(bmp);
      return b2;
   }
   else 
      return bmp;
}



/* saves a bitmap into the datafile format */
static void save_datafile_bitmap(DATAFILE *dat, int packed, int packkids, int strip, int verbose, int extra, PACKFILE *f)
{
   BITMAP *bmp = (BITMAP *)dat->dat;
   int x, y, c, r, g, b;
   unsigned short *p16;
   unsigned long *p32;
   int depth = bitmap_color_depth(bmp);

   pack_mputw(depth, f);
   pack_mputw(bmp->w, f);
   pack_mputw(bmp->h, f);

   switch (depth) {

      case 8:
	 /* 256 colors */
	 for (y=0; y<bmp->h; y++)
	    for (x=0; x<bmp->w; x++)
	       pack_putc(bmp->line[y][x], f);
	 break;

      case 15:
      case 16:
	 /* hicolor */
	 for (y=0; y<bmp->h; y++) {
	    p16 = (unsigned short *)bmp->line[y];

	    for (x=0; x<bmp->w; x++) {
	       c = p16[x];
	       r = getr_depth(depth, c);
	       g = getg_depth(depth, c);
	       b = getb_depth(depth, c);
	       c = ((r<<8)&0xF800) | ((g<<3)&0x7E0) | ((b>>3)&0x1F);
	       pack_iputw(c, f);
	    }
	 }
	 break;

      case 24:
	 /* 24 bit truecolor */
	 for (y=0; y<bmp->h; y++) {
	    for (x=0; x<bmp->w; x++) {
	       r = bmp->line[y][x*3+_rgb_r_shift_24/8];
	       g = bmp->line[y][x*3+_rgb_g_shift_24/8];
	       b = bmp->line[y][x*3+_rgb_b_shift_24/8];
	       pack_putc(r, f);
	       pack_putc(g, f);
	       pack_putc(b, f);
	    }
	 }
	 break;

      case 32:
	 /* 32 bit truecolor */
	 for (y=0; y<bmp->h; y++) {
	    p32 = (unsigned long *)bmp->line[y];

	    for (x=0; x<bmp->w; x++) {
	       c = p32[x];
	       r = getr32(c);
	       g = getg32(c);
	       b = getb32(c);
	       pack_putc(r, f);
	       pack_putc(g, f);
	       pack_putc(b, f);
	    }
	 }
	 break;
   }
}



/* creates a new RLE sprite object */
static void *makenew_rle_sprite(long *size)
{
   BITMAP *bmp = makenew_bitmap(size);
   RLE_SPRITE *spr = get_rle_sprite(bmp);

   destroy_bitmap(bmp);

   return spr;
}



/* draws an RLE sprite onto the grabber object view window */
static void plot_rle_sprite(DATAFILE *dat, int x, int y)
{
   RLE_SPRITE *s = dat->dat;
   BITMAP *b = create_bitmap_ex(s->color_depth, s->w, s->h);
   DATAFILE tmpdat = { b, DAT_RLE_SPRITE, 0, NULL };
   clear_to_color(b, b->vtable->mask_color);
   draw_rle_sprite(b, s, 0, 0);
   plot_bitmap(&tmpdat, x, y);
   destroy_bitmap(b);
}



/* handles double-clicking on an RLE sprite in the grabber */
static int view_rle_sprite(DATAFILE *dat)
{
   RLE_SPRITE *spr = dat->dat;
   BITMAP *bmp = create_bitmap_ex(spr->color_depth, spr->w, spr->h);
   DATAFILE tmpdat = { bmp, DAT_BITMAP, 0, NULL };
   clear_to_color(bmp, bmp->vtable->mask_color);
   draw_rle_sprite(bmp, spr, 0, 0);
   view_bitmap(&tmpdat);
   destroy_bitmap(bmp);
   return D_REDRAW;
}



/* returns a description string for an RLE sprite */
static void get_rle_sprite_desc(DATAFILE *dat, char *s)
{
   RLE_SPRITE *spr = (RLE_SPRITE *)dat->dat;

   sprintf(s, "RLE sprite (%dx%d, %d bit)", spr->w, spr->h, spr->color_depth);
}



/* exports an RLE sprite into an external file */
static int export_rle_sprite(DATAFILE *dat, char *filename)
{
   BITMAP *bmp;
   RLE_SPRITE *rle = (RLE_SPRITE *)dat->dat;
   int ret;

   bmp = create_bitmap_ex(rle->color_depth, rle->w, rle->h);
   clear_to_color(bmp, bmp->vtable->mask_color);
   draw_rle_sprite(bmp, rle, 0, 0);

   ret = (save_bitmap(filename, bmp, datedit_current_palette) == 0);

   destroy_bitmap(bmp);
   return ret;
}



/* reads an RLE sprite from an external file */
static void *grab_rle_sprite(char *filename, long *size, int x, int y, int w, int h, int depth)
{
   BITMAP *bmp;
   RLE_SPRITE *spr;

   bmp = (BITMAP *)grab_bitmap(filename, size, x, y, w, h, depth);
   if (!bmp)
      return NULL;

   spr = get_rle_sprite(bmp);
   destroy_bitmap(bmp);

   return spr;
}



/* saves an RLE sprite into the datafile format */
static void save_rle_sprite(DATAFILE *dat, int packed, int packkids, int strip, int verbose, int extra, PACKFILE *f)
{
   RLE_SPRITE *spr = (RLE_SPRITE *)dat->dat;
   int x, y, c, r, g, b;
   signed short *p16;
   signed long *p32;
   unsigned int eol_marker;
   int depth = spr->color_depth;

   pack_mputw(depth, f);
   pack_mputw(spr->w, f);
   pack_mputw(spr->h, f);
   pack_mputl(spr->size, f);

   switch (depth) {

      case 8:
	 /* 256 colors, easy! */
	 pack_fwrite(spr->dat, spr->size, f);
	 break;

      case 15:
      case 16:
	 /* hicolor */
	 p16 = (signed short *)spr->dat;
	 eol_marker = (depth == 15) ? MASK_COLOR_15 : MASK_COLOR_16;

	 for (y=0; y<spr->h; y++) {
	    while ((unsigned short)*p16 != eol_marker) {
	       if (*p16 < 0) {
		  /* skip count */
		  pack_iputw(*p16, f);
		  p16++;
	       }
	       else {
		  /* solid run */
		  x = *p16;
		  p16++;

		  pack_iputw(x, f);

		  while (x-- > 0) {
		     c = (unsigned short)*p16;
		     r = getr_depth(depth, c);
		     g = getg_depth(depth, c);
		     b = getb_depth(depth, c);
		     c = ((r<<8)&0xF800) | ((g<<3)&0x7E0) | ((b>>3)&0x1F);
		     pack_iputw(c, f);
		     p16++;
		  }
	       }
	    }

	    /* end of line */
	    pack_iputw(MASK_COLOR_16, f);
	    p16++;
	 }
	 break;

      case 24:
      case 32:
	 /* truecolor */
	 p32 = (signed long *)spr->dat;
	 eol_marker = (depth == 24) ? MASK_COLOR_24 : MASK_COLOR_32;

	 for (y=0; y<spr->h; y++) {
	    while ((unsigned long)*p32 != eol_marker) {
	       if (*p32 < 0) {
		  /* skip count */
		  pack_iputl(*p32, f);
		  p32++;
	       }
	       else {
		  /* solid run */
		  x = *p32;
		  p32++;

		  pack_iputl(x, f);

		  while (x-- > 0) {
		     c = (unsigned long)*p32;
		     r = getr_depth(depth, c);
		     g = getg_depth(depth, c);
		     b = getb_depth(depth, c);
		     pack_putc(r, f);
		     pack_putc(g, f);
		     pack_putc(b, f);
		     p32++;
		  }
	       }
	    }

	    /* end of line */
	    pack_iputl(MASK_COLOR_32, f);
	    p32++;
	 }
	 break;
   }
}



/* returns a description string for a compiled sprite object */
static void get_c_sprite_desc(DATAFILE *dat, char *s)
{
   BITMAP *bmp = (BITMAP *)dat->dat;

   sprintf(s, "compiled sprite (%dx%d, %d bit)", bmp->w, bmp->h, bitmap_color_depth(bmp));
}



/* returns a description string for a mode-X compiled sprite object */
static void get_xc_sprite_desc(DATAFILE *dat, char *s)
{
   BITMAP *bmp = (BITMAP *)dat->dat;

   if (bitmap_color_depth(bmp) == 8)
      sprintf(s, "mode-X compiled sprite (%dx%d)", bmp->w, bmp->h);
   else
      sprintf(s, "!!! %d bit XC sprite not possible !!!", bitmap_color_depth(bmp));
}



/* plugin interface header */
DATEDIT_OBJECT_INFO datbitmap_info =
{
   DAT_BITMAP, 
   "Bitmap", 
   get_bitmap_desc,
   makenew_bitmap,
   save_datafile_bitmap,
   plot_bitmap,
   view_bitmap,
   NULL
};



DATEDIT_GRABBER_INFO datbitmap_grabber =
{
   DAT_BITMAP, 
   "bmp;lbm;pcx;tga",
   "bmp;pcx;tga",
   grab_bitmap,
   export_bitmap
};



/* plugin interface header */
DATEDIT_OBJECT_INFO datrlesprite_info =
{
   DAT_RLE_SPRITE, 
   "RLE sprite", 
   get_rle_sprite_desc,
   makenew_rle_sprite,
   save_rle_sprite,
   plot_rle_sprite,
   view_rle_sprite,
   NULL
};



DATEDIT_GRABBER_INFO datrlesprite_grabber =
{
   DAT_RLE_SPRITE, 
   "bmp;lbm;pcx;tga",
   "bmp;pcx;tga",
   grab_rle_sprite,
   export_rle_sprite
};



/* plugin interface header */
DATEDIT_OBJECT_INFO datcsprite_info =
{
   DAT_C_SPRITE, 
   "Compiled sprite", 
   get_c_sprite_desc,
   makenew_bitmap,
   save_datafile_bitmap,
   plot_bitmap,
   view_bitmap,
   NULL
};



DATEDIT_GRABBER_INFO datcsprite_grabber =
{
   DAT_C_SPRITE, 
   "bmp;lbm;pcx;tga",
   "bmp;pcx;tga",
   grab_bitmap,
   export_bitmap
};



/* plugin interface header */
DATEDIT_OBJECT_INFO datxcsprite_info =
{ 
   DAT_XC_SPRITE, 
   "Compiled X-sprite", 
   get_xc_sprite_desc,
   makenew_bitmap,
   save_datafile_bitmap,
   plot_bitmap,
   view_bitmap,
   NULL
};



DATEDIT_GRABBER_INFO datxcsprite_grabber =
{ 
   DAT_XC_SPRITE, 
   "bmp;lbm;pcx;tga",
   "bmp;pcx;tga",
   grab_bitmap,
   export_bitmap
};


