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
 *      Grabber plugin for managing font objects.
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



/* creates a new font object */
static void *makenew_font(long *size)
{
   FONT *f = malloc(sizeof(FONT));

   f->height = 8;
   f->dat.dat_8x8 = malloc(sizeof(FONT_8x8));
   memcpy(f->dat.dat_8x8, font->dat.dat_8x8, sizeof(FONT_8x8));

   return f;
}



/* displays a font in the grabber object view window */
static void plot_font(DATAFILE *dat, int x, int y)
{
   textout(screen, dat->dat, " !\"#$%&'()*+,-./", x, y+32, gui_fg_color);
   textout(screen, dat->dat, "0123456789:;<=>?", x, y+64, gui_fg_color);
   textout(screen, dat->dat, "@ABCDEFGHIJKLMNO", x, y+96, gui_fg_color);
   textout(screen, dat->dat, "PQRSTUVWXYZ[\\]^_", x, y+128, gui_fg_color);
   textout(screen, dat->dat, "`abcdefghijklmno", x, y+160, gui_fg_color);
   textout(screen, dat->dat, "pqrstuvwxyz{|}~", x, y+192, gui_fg_color);
}



/* handles double-clicking on a font in the grabber */
static int view_font(DATAFILE *dat)
{
   char buf[2];
   int page = 0;
   int c, x, y;
   FONT *f = dat->dat;

   show_mouse(NULL);
   text_mode(-1);
   buf[1] = 0;

   for (;;) {
      clear_to_color(screen, gui_mg_color);

      for (c=0; c<128; c++) {
	 buf[0] = page+c;
	 x = (c&15) * SCREEN_W / 16;
	 y = (c/16) * SCREEN_H / 8;
	 textout(screen, font, buf, x, y, gui_bg_color);
	 textout(screen, f, buf, x+8, y+8, gui_fg_color);
      }

      do {
      } while (mouse_b);

      clear_keybuf();

      do {
      } while ((!mouse_b) && (!keypressed()));

      if (keypressed()) {
	 if ((readkey() & 0xFF) == 27)
	    break;
      }
      else {
	 if (mouse_b & 2)
	    break;
      }

      page = 128-page;
   }

   do {
   } while (mouse_b);
   clear_keybuf();

   show_mouse(screen);

   return D_REDRAW;
}



/* returns a description string for a font object */
static void get_font_desc(DATAFILE *dat, char *s)
{
   FONT *font = (FONT *)dat->dat;

   if (font->height < 0)
      strcpy(s, "proportional font");
   else
      sprintf(s, "8x%d font", font->height);
}



/* exports a font into an external file */
static int export_font(DATAFILE *dat, char *filename)
{
   FONT *font = (FONT *)dat->dat;
   FONT_PROP *font_prop = NULL;
   BITMAP *b;
   char buf[2];
   int w, h, c;

   w = 0;
   h = 0;

   if (font->height < 0) {
      font_prop = font->dat.dat_prop;

      for (c=0; c<FONT_SIZE; c++) {
	 if (font_prop->dat[c]->w > w)
	    w = font_prop->dat[c]->w;
	 if (font_prop->dat[c]->h > h)
	    h = font_prop->dat[c]->h;
      }
   }
   else {
      w = 8;
      h = font->height;
   }

   w = (w+16) & 0xFFF0;
   h = (h+16) & 0xFFF0;

   b = create_bitmap_ex(8, 1+w*16, 1+h*((FONT_SIZE+15)/16));
   rectfill(b, 0, 0, b->w, b->h, 255);
   text_mode(0);

   for (c=0; c<FONT_SIZE; c++) {
      buf[0] = c + ' ';
      buf[1] = 0;

      textout(b, font, buf, 1+w*(c&15), 1+h*(c/16), font_prop ? -1 : 1);
   }

   save_bitmap(filename, b, desktop_pallete);
   destroy_bitmap(b);

   return (errno == 0);
}



/* GRX font file reader by Mark Wodrich.
 *
 * GRX FNT files consist of the header data (see struct below). If the font
 * is proportional, followed by a table of widths per character (unsigned 
 * shorts). Then, the data for each character follows. 1 bit/pixel is used,
 * with each line of the character stored in contiguous bytes. High bit of
 * first byte is leftmost pixel of line.
 *
 * Note : FNT files can have a variable number of characters, so we must
 *        check that the chars 32..127 exist.
 */


#define FONTMAGIC       0x19590214L


/* .FNT file header */
typedef struct {
   unsigned long  magic;
   unsigned long  bmpsize;
   unsigned short width;
   unsigned short height;
   unsigned short minchar;
   unsigned short maxchar;
   unsigned short isfixed;
   unsigned short reserved;
   unsigned short baseline;
   unsigned short undwidth;
   char           fname[16];
   char           family[16];
} FNTfile_header;


#define GRX_TMP_SIZE    4096



/* converts images from bit to byte format */
static void convert_grx_bitmap(int width, int height, unsigned char *src, unsigned char *dest) 
{
   unsigned short x, y, bytes_per_line;
   unsigned char bitpos, bitset;

   bytes_per_line = (width+7) >> 3;

   for (y=0; y<height; y++) {
      for (x=0; x<width; x++) {
	 bitpos = 7-(x&7);
	 bitset = !!(src[(bytes_per_line*y) + (x>>3)] & (1<<bitpos));
	 dest[y*width+x] = bitset;
      }
   }
}



/* reads GRX format images from disk */
static unsigned char **load_grx_bmps(PACKFILE *f, FNTfile_header *hdr, int numchar, unsigned short *wtable) 
{
   int t, width, bmp_size;
   unsigned char *temp;
   unsigned char **bmp;

   /* alloc array of bitmap pointers */
   bmp = malloc(sizeof(unsigned char *) * numchar);

   /* assume it's fixed width for now */
   width = hdr->width;

   /* temporary working area to store FNT bitmap */
   temp = malloc(GRX_TMP_SIZE);

   for (t=0; t<numchar; t++) {
      /* if prop. get character width */
      if (!hdr->isfixed) 
	 width = wtable[t];

      /* work out how many bytes to read */
      bmp_size = ((width+7) >> 3) * hdr->height;

      /* oops, out of space! */
      if (bmp_size > GRX_TMP_SIZE) {
	 free(temp);
	 for (t--; t>=0; t--)
	    free(bmp[t]);
	 free(bmp);
	 return NULL;
      }

      /* alloc space for converted bitmap */
      bmp[t] = malloc(width*hdr->height);

      /* read data */
      pack_fread(temp, bmp_size, f);

      /* convert to 1 byte/pixel */
      convert_grx_bitmap(width, hdr->height, temp, bmp[t]);
   }

   free(temp);
   return bmp;
}



/* main import routine for the GRX font format */
static FONT *import_grx_font(char *fname)
{
   PACKFILE *f, *cf;
   FNTfile_header hdr;              /* GRX font header */
   int numchar;                     /* number of characters in the font */
   unsigned short *wtable = NULL;   /* table of widths for each character */
   unsigned char **bmp;             /* array of font bitmaps */
   FONT *font = NULL;               /* the Allegro font */
   FONT_PROP *font_prop;
   int c, c2, start, width;
   char copyright[256];

   f = pack_fopen(fname, F_READ);
   if (!f)
      return NULL;

   pack_fread(&hdr, sizeof(hdr), f);      /* read the header structure */

   if (hdr.magic != FONTMAGIC) {          /* check magic number */
      pack_fclose(f);
      return NULL;
   }

   numchar = hdr.maxchar-hdr.minchar+1;

   if (!hdr.isfixed) {                    /* proportional font */
      wtable = malloc(sizeof(unsigned short) * numchar);
      pack_fread(wtable, sizeof(unsigned short) * numchar, f);
   }

   bmp = load_grx_bmps(f, &hdr, numchar, wtable);
   if (!bmp)
      goto get_out;

   if (pack_ferror(f))
      goto get_out;

   if ((hdr.minchar < ' ') || (hdr.maxchar >= ' '+FONT_SIZE))
      datedit_msg("Warning: font exceeds range 32..256. Characters will be lost in conversion");

   font = malloc(sizeof(FONT));
   font->height = -1;
   font->dat.dat_prop = font_prop = malloc(sizeof(FONT_PROP));
   font_prop->render = NULL;

   start = 32 - hdr.minchar;
   width = hdr.width;

   for (c=0; c<FONT_SIZE; c++) {
      c2 = c+start;

      if ((c2 >= 0) && (c2 < numchar)) {
	 if (!hdr.isfixed)
	    width = wtable[c2];

	 font_prop->dat[c] = create_bitmap_ex(8, width, hdr.height);
	 memcpy(font_prop->dat[c]->dat, bmp[c2], width*hdr.height);
      }
      else {
	 font_prop->dat[c] = create_bitmap_ex(8, 8, hdr.height);
	 clear(font_prop->dat[c]);
      }
   }

   if (!pack_feof(f)) {
      strcpy(copyright, fname);
      strcpy(get_extension(copyright), "txt");
      c = datedit_ask("Save font copyright message into '%s'", copyright);
      if ((c != 27) && (c != 'n') && (c != 'N')) {
	 cf = pack_fopen(copyright, F_WRITE);
	 if (cf) {
	    while (!pack_feof(f)) {
	       pack_fgets(copyright, 255, f);
	       if (isspace(copyright[0])) {
		  pack_fputs(copyright, cf);
		  pack_fputs("\n", cf);
	       }
	       else if (!copyright[0])
		  pack_fputs("\n", cf);
	    }
	    pack_fclose(cf);
	 }
      }
   }

   get_out:

   pack_fclose(f);

   if (wtable)
      free(wtable);

   if (bmp) {
      for (c=0; c<numchar; c++)
	 free(bmp[c]);
      free(bmp);
   }

   return font;
}



/* main import routine for the 8x8 and 8x16 BIOS font formats */
static FONT *import_bios_font(char *fname)
{
   PACKFILE *f;
   FONT *font = NULL; 
   unsigned char data16[256][16];
   unsigned char data8[256][8];
   int font_h;
   int c, x, y;

   f = pack_fopen(fname, F_READ);
   if (!f)
      return NULL;

   font_h = (f->todo == 2048) ? 8 : 16;

   if (font_h == 16)
      pack_fread(data16, sizeof(data16), f);
   else
      pack_fread(data8, sizeof(data8), f);

   pack_fclose(f);

   font = malloc(sizeof(FONT));
   font->height = -1;
   font->dat.dat_prop = malloc(sizeof(FONT_PROP));
   font->dat.dat_prop->render = NULL;

   for (c=0; c<FONT_SIZE; c++) {
      font->dat.dat_prop->dat[c] = create_bitmap_ex(8, 8, font_h);
      clear(font->dat.dat_prop->dat[c]);

      for (y=0; y<font_h; y++) {
	 for (x=0; x<8; x++) {
	    if (font_h == 16) {
	       if (data16[c + ' '][y] & (0x80 >> x))
		  font->dat.dat_prop->dat[c]->line[y][x] = 1;
	    }
	    else {
	       if (data8[c + ' '][y] & (0x80 >> x))
		  font->dat.dat_prop->dat[c]->line[y][x] = 1;
	    }
	 }
      }
   }

   return font;
}



/* main import routine for the Allegro .pcx font format */
static void *import_bitmap_font(char *fname)
{
   PALLETE junk;
   BITMAP *bmp;
   FONT *f;
   int x, y, w, h, c;
   int max_h = 0;

   bmp = load_bitmap(fname, junk);
   if (!bmp)
      return NULL;

   if (bitmap_color_depth(bmp) != 8) {
      destroy_bitmap(bmp);
      return NULL;
   }

   f = malloc(sizeof(FONT));
   f->height = -1;
   f->dat.dat_prop = malloc(sizeof(FONT_PROP));
   f->dat.dat_prop->render = NULL;
   for (c=0; c<FONT_SIZE; c++)
      f->dat.dat_prop->dat[c] = NULL;

   x = 0;
   y = 0;

   for (c=0; c<FONT_SIZE; c++) {
      datedit_find_character(bmp, &x, &y, &w, &h);

      if ((w <= 0) || (h <= 0)) {
	 w = 8;
	 h = 8;
      }

      f->dat.dat_prop->dat[c] = create_bitmap_ex(8, w, h);
      clear(f->dat.dat_prop->dat[c]);
      blit(bmp, f->dat.dat_prop->dat[c], x+1, y+1, 0, 0, w, h);

      max_h = MAX(max_h, h);
      x += w;
   }

   for (c=0; c<FONT_SIZE; c++) {
      if (f->dat.dat_prop->dat[c]->h < max_h) {
	 BITMAP *b = f->dat.dat_prop->dat[c];
	 f->dat.dat_prop->dat[c] = create_bitmap_ex(8, b->w, max_h);
	 clear(f->dat.dat_prop->dat[c]);
	 blit(b, f->dat.dat_prop->dat[c], 0, 0, 0, 0, b->w, b->h);
	 destroy_bitmap(b);
      }
   }

   destroy_bitmap(bmp);
   return f;
}



/* converts a proportional font to 8x8 format */
static FONT *make_font8x8(FONT *f)
{
   FONT_PROP *fp = f->dat.dat_prop;
   FONT_8x8 *f8 = malloc(sizeof(FONT_8x8));
   BITMAP *bmp;
   int c, x, y;

   for (c=0; c<FONT_SIZE; c++) {
      bmp = fp->dat[c];

      for (y=0; y<8; y++) {
	 f8->dat[c][y] = 0;
	 for (x=0; x<8; x++)
	    if (bmp->line[y][x])
	       f8->dat[c][y] |= (0x80 >> x);
      }

      destroy_bitmap(bmp); 
   }

   free(fp);
   f->dat.dat_8x8 = f8;
   f->height = 8;
   return f;
}



/* converts a proportional font to 8x16 format */
static FONT *make_font8x16(FONT *f)
{
   FONT_PROP *fp = f->dat.dat_prop;
   FONT_8x16 *f16 = malloc(sizeof(FONT_8x16));
   BITMAP *bmp;
   int c, x, y;

   for (c=0; c<FONT_SIZE; c++) {
      bmp = fp->dat[c];

      for (y=0; y<16; y++) {
	 f16->dat[c][y] = 0;
	 for (x=0; x<8; x++)
	    if (bmp->line[y][x])
	       f16->dat[c][y] |= (0x80 >> x);
      }

      destroy_bitmap(bmp); 
   }

   free(fp);
   f->dat.dat_8x16 = f16;
   f->height = 16;
   return f;
}



/* make sure a font will use 8x8 or 8x16 format if that is possible */
static FONT *fixup_font(FONT *f)
{
   int c, w, h, x, y, n;
   int col = -1;

   if (!f)
      return NULL;

   w = f->dat.dat_prop->dat[0]->w;
   h = f->dat.dat_prop->dat[0]->h;

   for (c=1; c<FONT_SIZE; c++) {
      if ((f->dat.dat_prop->dat[c]->w != w) ||
	  (f->dat.dat_prop->dat[c]->h != h))
	 return f;

      for (y=0; y<h; y++) {
	 for (x=0; x<w; x++) {
	    n = f->dat.dat_prop->dat[c]->line[y][x];
	    if (n) {
	       if (col < 0)
		  col = n;
	       else if (col != n)
		  return f;
	    }
	 }
      }
   }

   if ((w == 8) && (h == 8))
      return make_font8x8(f);
   else if ((w == 8) && (h == 16))
      return make_font8x16(f);
   else
      return f;
}



/* imports a font from an external file (handles various formats) */
static void *grab_font(char *filename, long *size, int x, int y, int w, int h, int depth)
{
   PACKFILE *f;
   int id;

   if ((stricmp(get_extension(filename), "bmp") == 0) ||
       (stricmp(get_extension(filename), "lbm") == 0) ||
       (stricmp(get_extension(filename), "pcx") == 0) ||
       (stricmp(get_extension(filename), "tga") == 0)) {
      return fixup_font(import_bitmap_font(filename));
   }
   else {
      f = pack_fopen(filename, F_READ);
      if (!f)
	 return NULL;

      id = pack_igetl(f);
      pack_fclose(f);

      if (id == FONTMAGIC)
	 return fixup_font(import_grx_font(filename));
      else
	 return fixup_font(import_bios_font(filename));
   }
}



/* saves a font into a datafile */
static void save_font(DATAFILE *dat, int packed, int packkids, int strip, int verbose, int extra, PACKFILE *f)
{
   FONT *font = (FONT *)dat->dat;
   BITMAP *bmp;
   int c, x, y;

   pack_mputw(font->height, f);

   if (font->height > 0) {
      if (font->height == 8)
	 pack_fwrite(font->dat.dat_8x8, sizeof(FONT_8x8), f);
      else
	 pack_fwrite(font->dat.dat_8x16, sizeof(FONT_8x16), f);
   }
   else {
      for (c=0; c<FONT_SIZE; c++) {
	 bmp = font->dat.dat_prop->dat[c];

	 pack_mputw(bmp->w, f);
	 pack_mputw(bmp->h, f);

	 for (y=0; y<bmp->h; y++)
	    for (x=0; x<bmp->w; x++)
	       pack_putc(bmp->line[y][x], f);
      }
   }
}



/* plugin interface header */
DATEDIT_OBJECT_INFO datfont_info =
{
   DAT_FONT, 
   "Font", 
   get_font_desc,
   makenew_font,
   save_font,
   plot_font,
   view_font,
   NULL
};



DATEDIT_GRABBER_INFO datfont_grabber =
{
   DAT_FONT, 
   "fnt;bmp;lbm;pcx;tga",
   "bmp;pcx;tga",
   grab_font,
   export_font
};

