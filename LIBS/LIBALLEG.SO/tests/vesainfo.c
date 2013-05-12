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
 *      Allegro library utility for checking which VESA modes are available.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/movedata.h>
#include <sys/farptr.h>
#include <sys/segments.h>

#include "allegro.h"



int verbose = FALSE;


#define MASK_LINEAR(addr)     (addr & 0x000FFFFF)
#define RM_TO_LINEAR(addr)    (((addr & 0xFFFF0000) >> 12) + (addr & 0xFFFF))
#define RM_OFFSET(addr)       (addr & 0xF)
#define RM_SEGMENT(addr)      ((addr >> 4) & 0xFFFF)



typedef struct VESA_INFO 
{ 
   unsigned char  VESASignature[4]     __attribute__ ((packed));
   unsigned short VESAVersion          __attribute__ ((packed));
   unsigned long  OEMStringPtr         __attribute__ ((packed));
   unsigned char  Capabilities[4]      __attribute__ ((packed));
   unsigned long  VideoModePtr         __attribute__ ((packed)); 
   unsigned short TotalMemory          __attribute__ ((packed)); 
   unsigned short OemSoftwareRev       __attribute__ ((packed)); 
   unsigned long  OemVendorNamePtr     __attribute__ ((packed)); 
   unsigned long  OemProductNamePtr    __attribute__ ((packed)); 
   unsigned long  OemProductRevPtr     __attribute__ ((packed)); 
   unsigned char  Reserved[222]        __attribute__ ((packed)); 
   unsigned char  OemData[256]         __attribute__ ((packed)); 
} VESA_INFO;



typedef struct MODE_INFO 
{
   unsigned short ModeAttributes       __attribute__ ((packed)); 
   unsigned char  WinAAttributes       __attribute__ ((packed)); 
   unsigned char  WinBAttributes       __attribute__ ((packed)); 
   unsigned short WinGranularity       __attribute__ ((packed)); 
   unsigned short WinSize              __attribute__ ((packed)); 
   unsigned short WinASegment          __attribute__ ((packed)); 
   unsigned short WinBSegment          __attribute__ ((packed)); 
   unsigned long  WinFuncPtr           __attribute__ ((packed)); 
   unsigned short BytesPerScanLine     __attribute__ ((packed)); 
   unsigned short XResolution          __attribute__ ((packed)); 
   unsigned short YResolution          __attribute__ ((packed)); 
   unsigned char  XCharSize            __attribute__ ((packed)); 
   unsigned char  YCharSize            __attribute__ ((packed)); 
   unsigned char  NumberOfPlanes       __attribute__ ((packed)); 
   unsigned char  BitsPerPixel         __attribute__ ((packed)); 
   unsigned char  NumberOfBanks        __attribute__ ((packed)); 
   unsigned char  MemoryModel          __attribute__ ((packed)); 
   unsigned char  BankSize             __attribute__ ((packed)); 
   unsigned char  NumberOfImagePages   __attribute__ ((packed));
   unsigned char  Reserved_page        __attribute__ ((packed)); 
   unsigned char  RedMaskSize          __attribute__ ((packed)); 
   unsigned char  RedMaskPos           __attribute__ ((packed)); 
   unsigned char  GreenMaskSize        __attribute__ ((packed)); 
   unsigned char  GreenMaskPos         __attribute__ ((packed));
   unsigned char  BlueMaskSize         __attribute__ ((packed)); 
   unsigned char  BlueMaskPos          __attribute__ ((packed)); 
   unsigned char  ReservedMaskSize     __attribute__ ((packed)); 
   unsigned char  ReservedMaskPos      __attribute__ ((packed)); 
   unsigned char  DirectColorModeInfo  __attribute__ ((packed));
   unsigned long  PhysBasePtr          __attribute__ ((packed)); 
   unsigned long  OffScreenMemOffset   __attribute__ ((packed)); 
   unsigned short OffScreenMemSize     __attribute__ ((packed)); 
   unsigned short LinBytesPerScanLine  __attribute__ ((packed));
   unsigned char  BnkNumberOfPages     __attribute__ ((packed));
   unsigned char  LinNumberOfPages     __attribute__ ((packed));
   unsigned char  LinRedMaskSize       __attribute__ ((packed));
   unsigned char  LinRedFieldPos       __attribute__ ((packed));
   unsigned char  LinGreenMaskSize     __attribute__ ((packed));
   unsigned char  LinGreenFieldPos     __attribute__ ((packed));
   unsigned char  LinBlueMaskSize      __attribute__ ((packed));
   unsigned char  LinBlueFieldPos      __attribute__ ((packed));
   unsigned char  LinRsvdMaskSize      __attribute__ ((packed));
   unsigned char  LinRsvdFieldPos      __attribute__ ((packed));
   unsigned long  MaxPixelClock        __attribute__ ((packed));
   unsigned char  Reserved[190]        __attribute__ ((packed));
} MODE_INFO;



VESA_INFO vesa_info;
MODE_INFO mode_info;


#define MAX_VESA_MODES 1024

unsigned short mode[MAX_VESA_MODES];
int modes;


char oem_string[256];
char oemvendor_string[256];
char oemproductname_string[256];
char oemproductrev_string[256];



void get_string(char *s, unsigned long addr)
{
   if (addr) {
      addr = RM_TO_LINEAR(addr);
      while (_farnspeekb(addr) != 0)
	 *(s++) = _farnspeekb(addr++);
      *s = 0;
   }
   else
      strcpy(s, "(null)");
}



int get_vesa_info()
{
   __dpmi_regs r;
   unsigned long mode_ptr;
   int c;

   for (c=0; c<(int)sizeof(VESA_INFO); c++)
      _farpokeb(_dos_ds, MASK_LINEAR(__tb)+c, 0);

   dosmemput("VBE2", 4, MASK_LINEAR(__tb));

   r.x.ax = 0x4F00;
   r.x.di = RM_OFFSET(__tb);
   r.x.es = RM_SEGMENT(__tb);
   __dpmi_int(0x10, &r);
   if (r.h.ah) {
      printf("Int 0x10, ax=0x4F00 failed! ax=0x%02X\n", r.x.ax);
      return -1;
   }

   dosmemget(MASK_LINEAR(__tb), sizeof(VESA_INFO), &vesa_info);

   if (strncmp(vesa_info.VESASignature, "VESA", 4) != 0) {
      printf("Bad VESA signature! (%4.4s)\n", vesa_info.VESASignature);
      return -1;
   }

   _farsetsel(_dos_ds);

   modes = 0;
   mode_ptr = RM_TO_LINEAR(vesa_info.VideoModePtr);
   mode[modes] = _farnspeekw(mode_ptr);

   while (mode[modes] != 0xFFFF) {
      modes++;
      mode_ptr += 2;
      mode[modes] = _farnspeekw(mode_ptr);
   }

   get_string(oem_string, vesa_info.OEMStringPtr);
   get_string(oemvendor_string, vesa_info.OemVendorNamePtr);
   get_string(oemproductname_string, vesa_info.OemProductNamePtr);
   get_string(oemproductrev_string, vesa_info.OemProductRevPtr);

   printf("\n");

   printf("VESASignature:\t\t%4.4s\n", vesa_info.VESASignature);
   printf("VESAVersion:\t\t%d.%d\n", vesa_info.VESAVersion>>8, vesa_info.VESAVersion&0xFF);
   printf("OEMStringPtr:\t\t%s\n", oem_string);

   if (verbose) {
      printf("Capabilities:\t\t%d (%s DAC | %s compatible | %s RAMDAC)\n", 
	     vesa_info.Capabilities[0],
	     (vesa_info.Capabilities[0] & 1) ? "switchable" : "fixed",
	     (vesa_info.Capabilities[0] & 2) ? "not VGA" : "VGA",
	     (vesa_info.Capabilities[0] & 4) ? "vblank" : "normal");
   }

   printf("TotalMemory:\t\t%d (%dK)\n", vesa_info.TotalMemory, vesa_info.TotalMemory*64);

   if (verbose) {
      printf("OemSoftwareRev:\t\t%d.%d\n", vesa_info.OemSoftwareRev>>8, vesa_info.OemSoftwareRev&0xFF);
      printf("OemVendorNamePtr:\t%s\n", oemvendor_string);
      printf("OemProductNamePtr:\t%s\n", oemproductname_string);
      printf("OemProductRevPtr:\t%s\n", oemproductrev_string);

      printf("\n\n\n");
   }
   else
      printf("\n");

   return 0;
}



int in_flag = FALSE;



void print_flag(char *s)
{
   if (!in_flag) {
      printf(" (");
      in_flag = TRUE;
   }
   else
      printf(" | ");

   printf(s);
}



void end_print_flag()
{
   if (in_flag) {
      printf(")\n");
      in_flag = FALSE;
   }
   else
      printf("\n");
}



void print_window_attributes(int attrib)
{ 
   if (attrib & 1)
      print_flag("moveable");

   if (attrib & 2) 
      print_flag("readable");

   if (attrib & 4)
      print_flag("writable");

   end_print_flag();
}



typedef struct COLORINFO
{
   int size;
   int pos;
   char c;
} COLORINFO;

COLORINFO colorinfo[4];

int colorcount = 0;



int color_cmp(const void *e1, const void *e2)
{
   COLORINFO *c1 = (COLORINFO *)e1;
   COLORINFO *c2 = (COLORINFO *)e2;

   return c2->pos - c1->pos;
}



void add_color(int size, int pos, char c)
{
   if ((size > 0) || (pos > 0)) {
      colorinfo[colorcount].size = size;
      colorinfo[colorcount].pos = pos;
      colorinfo[colorcount].c = c;
      colorcount++;
   }
}



void get_color_desc(char *s)
{
   int c;

   if (colorcount > 0) {
      qsort(colorinfo, colorcount, sizeof(COLORINFO), color_cmp);

      for (c=0; c<colorcount; c++)
	 *(s++) = colorinfo[c].c;

      *(s++) = ' ';

      for (c=0; c<colorcount; c++)
	 *(s++) = '0' + colorinfo[c].size;

      colorcount = 0;
   }

   *s = 0;
}



int get_mode_info(int mode)
{
   __dpmi_regs r;
   char color_desc[80];
   int c;

   if (verbose)
      printf("Mode 0x%X:\n\n", mode);
   else
      printf("Mode 0x%-3X: ", mode);

   for (c=0; c<(int)sizeof(MODE_INFO); c++)
      _farpokeb(_dos_ds, MASK_LINEAR(__tb)+c, 0);

   r.x.ax = 0x4F01;
   r.x.di = RM_OFFSET(__tb);
   r.x.es = RM_SEGMENT(__tb);
   r.x.cx = mode;
   __dpmi_int(0x10, &r);
   if (r.h.ah) {
      printf("Int 0x10, ax=0x4F01 failed! ax=0x%02X\n", r.x.ax);
      if (verbose)
	 printf("\n\n\n");
      return -1;
   }

   dosmemget(MASK_LINEAR(__tb), sizeof(MODE_INFO), &mode_info);

   add_color(mode_info.RedMaskSize, mode_info.RedMaskPos, 'R');
   add_color(mode_info.GreenMaskSize, mode_info.GreenMaskPos, 'G');
   add_color(mode_info.BlueMaskSize, mode_info.BlueMaskPos, 'B');
   add_color(mode_info.ReservedMaskSize, mode_info.ReservedMaskPos, 'x');
   get_color_desc(color_desc);

   if (verbose) {
      printf("ModeAttributes:\t\t0x%X", mode_info.ModeAttributes);
      if (mode_info.ModeAttributes & 1) print_flag("ok");
      if (mode_info.ModeAttributes & 4) print_flag("tty");
      if (mode_info.ModeAttributes & 8) print_flag("col"); else print_flag("mono");
      if (mode_info.ModeAttributes & 16) print_flag("gfx"); else print_flag("text");
      if (!(mode_info.ModeAttributes & 32)) print_flag("VGA");
      if (!(mode_info.ModeAttributes & 64)) print_flag("bank");
      if (mode_info.ModeAttributes & 128) print_flag("lfb");
      if (mode_info.ModeAttributes & 1024) print_flag("3buf");
      end_print_flag();

      printf("WinAAttributes:\t\t%d", mode_info.WinAAttributes);
      print_window_attributes(mode_info.WinAAttributes);

      printf("WinBAttributes:\t\t%d", mode_info.WinBAttributes);
      print_window_attributes(mode_info.WinBAttributes);

      printf("WinGranularity:\t\t%dK\n", mode_info.WinGranularity);
      printf("WinSize:\t\t%dK\n", mode_info.WinSize);
      printf("WinASegment:\t\t%X\n", mode_info.WinASegment);
      printf("WinBSegment:\t\t%X\n", mode_info.WinBSegment);
      printf("BytesPerScanLine:\t%d\n", mode_info.BytesPerScanLine);
      printf("XResolution:\t\t%d\n", mode_info.XResolution);
      printf("YResolution:\t\t%d\n", mode_info.YResolution);
      printf("XCharSize:\t\t%d\n", mode_info.XCharSize);
      printf("YCharSize:\t\t%d\n", mode_info.YCharSize);
      printf("NumberOfPlanes:\t\t%d\n", mode_info.NumberOfPlanes);
      printf("BitsPerPixel:\t\t%d\n", mode_info.BitsPerPixel);
      printf("NumberOfBanks:\t\t%d\n", mode_info.NumberOfBanks);

      printf("MemoryModel:\t\t%d (", mode_info.MemoryModel);
      switch (mode_info.MemoryModel) {
	 case 0:  printf("text"); break;
	 case 1:  printf("CGA"); break;
	 case 2:  printf("Hercules"); break;
	 case 3:  printf("planar"); break;
	 case 4:  printf("packed pixel"); break;
	 case 5:  printf("mode-X"); break;
	 case 6:  printf("direct color"); break;
	 case 7:  printf("YUV"); break;
	 default: printf("unknown"); break;
      }
      if (color_desc[0])
	 printf(" %s", color_desc);
      printf(")\n");

      printf("BankSize:\t\t%d\n", mode_info.BankSize);
      printf("NumberOfImagePages:\t%d\n", mode_info.NumberOfImagePages);
      printf("Reserved_page:\t\t%d\n", mode_info.Reserved_page);
      printf("RedMaskSize:\t\t%d\n", mode_info.RedMaskSize);
      printf("RedMaskPos:\t\t%d\n", mode_info.RedMaskPos);
      printf("GreenMaskSize:\t\t%d\n", mode_info.GreenMaskSize);
      printf("GreenMaskPos:\t\t%d\n", mode_info.GreenMaskPos);
      printf("BlueMaskSize:\t\t%d\n", mode_info.BlueMaskSize);
      printf("BlueMaskPos:\t\t%d\n", mode_info.BlueMaskPos);
      printf("ReservedMaskSize:\t%d\n", mode_info.ReservedMaskSize);
      printf("ReservedMaskPos:\t%d\n", mode_info.ReservedMaskPos);

      printf("DirectColorModeInfo:\t%d (%s color ramp | %s reserved bits)\n", 
	     mode_info.DirectColorModeInfo,
	     (mode_info.DirectColorModeInfo & 1) ? "programmable" : "fixed",
	     (mode_info.DirectColorModeInfo & 2) ? "can use" : "don't use");

      printf("PhysBasePtr:\t\t0x%08lX\n", mode_info.PhysBasePtr);
      printf("OffScreenMemOffset:\t0x%08lX\n", mode_info.OffScreenMemOffset);
      printf("OffScreenMemSize:\t%dK\n", mode_info.OffScreenMemSize);

      if (vesa_info.VESAVersion >= 0x300) {
	 printf("LinBytesPerScanLine:\t%d\n", mode_info.LinBytesPerScanLine);
	 printf("BnkNumberOfPages:\t%d\n", mode_info.BnkNumberOfPages);
	 printf("LinNumberOfPages:\t%d\n", mode_info.LinNumberOfPages);
	 printf("LinRedMaskSize:\t\t%d\n", mode_info.LinRedMaskSize);
	 printf("LinRedFieldPos:\t\t%d\n", mode_info.LinRedFieldPos);
	 printf("LinGreenMaskSize:\t%d\n", mode_info.LinGreenMaskSize);
	 printf("LinGreenFieldPos:\t%d\n", mode_info.LinGreenFieldPos);
	 printf("LinBlueMaskSize:\t%d\n", mode_info.LinBlueMaskSize);
	 printf("LinBlueFieldPos:\t%d\n", mode_info.LinBlueFieldPos);
	 printf("LinRsvdMaskSize:\t%d\n", mode_info.LinRsvdMaskSize);
	 printf("LinRsvdFieldPos:\t%d\n", mode_info.LinRsvdFieldPos);
	 printf("MaxPixelClock:\t\t%ld\n", mode_info.MaxPixelClock);
      }

      printf("\n\n\n");
   }
   else {
      printf("%5dx%-6d", mode_info.XResolution, mode_info.YResolution);
      printf("%d bpp ", mode_info.BitsPerPixel);
      switch (mode_info.MemoryModel) {
	 case 0:  printf("text"); break;
	 case 1:  printf("CGA"); break;
	 case 2:  printf("Hercules"); break;
	 case 3:  printf("planar"); break;
	 case 4:  printf("packed pixel"); break;
	 case 5:  printf("non-chain 4, 256 color"); break;
	 case 6:  printf("direct color"); break;
	 case 7:  printf("YUV"); break;
	 default: printf("unknown"); break;
      }
      if (color_desc[0])
	 printf(" (%s)", color_desc);
      printf("\n");
   }

   return 0;
}



int main(int argc, char *argv[])
{
   int c;

   for (c=1; c<argc; c++)
      if (((argv[c][0] == '-') || (argv[c][0] == '/')) &&
	  ((argv[c][1] == 'v') || (argv[c][1] == 'V')))
	 verbose = TRUE;

   printf("\nAllegro VESAINFO utility " ALLEGRO_VERSION_STR);
   printf("\nBy Shawn Hargreaves, " ALLEGRO_DATE_STR "\n");
   if (verbose)
      printf("\n");

   if (get_vesa_info() != 0)
      return -1;

   for (c=0; c<modes; c++)
      get_mode_info(mode[c]);

   if (!verbose)
      printf("\n'vesainfo -v' for a verbose listing\n\n");

   return 0;
}

