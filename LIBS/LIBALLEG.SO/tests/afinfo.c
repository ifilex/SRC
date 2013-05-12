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
 *      Allegro library utility for checking which VBE/AF modes are available.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>
#include <sys/nearptr.h>

#include "internal.h"



int verbose = FALSE;



typedef struct AF_MODE_INFO
{
   unsigned short Attributes              __attribute__ ((packed));
   unsigned short XResolution             __attribute__ ((packed));
   unsigned short YResolution             __attribute__ ((packed));
   unsigned short BytesPerScanLine        __attribute__ ((packed));
   unsigned short BitsPerPixel            __attribute__ ((packed));
   unsigned short MaxBuffers              __attribute__ ((packed));
   unsigned char  RedMaskSize             __attribute__ ((packed));
   unsigned char  RedFieldPosition        __attribute__ ((packed));
   unsigned char  GreenMaskSize           __attribute__ ((packed));
   unsigned char  GreenFieldPosition      __attribute__ ((packed));
   unsigned char  BlueMaskSize            __attribute__ ((packed));
   unsigned char  BlueFieldPosition       __attribute__ ((packed));
   unsigned char  RsvdMaskSize            __attribute__ ((packed));
   unsigned char  RsvdFieldPosition       __attribute__ ((packed));
   unsigned short MaxBytesPerScanLine     __attribute__ ((packed));
   unsigned short MaxScanLineWidth        __attribute__ ((packed));
   unsigned short LinBytesPerScanLine     __attribute__ ((packed));
   unsigned char  BnkMaxBuffers           __attribute__ ((packed));
   unsigned char  LinMaxBuffers           __attribute__ ((packed));
   unsigned char  LinRedMaskSize          __attribute__ ((packed));
   unsigned char  LinRedFieldPosition     __attribute__ ((packed));
   unsigned char  LinGreenMaskSize        __attribute__ ((packed));
   unsigned char  LinGreenFieldPosition   __attribute__ ((packed));
   unsigned char  LinBlueMaskSize         __attribute__ ((packed));
   unsigned char  LinBlueFieldPosition    __attribute__ ((packed));
   unsigned char  LinRsvdMaskSize         __attribute__ ((packed));
   unsigned char  LinRsvdFieldPosition    __attribute__ ((packed));
   unsigned long  MaxPixelClock           __attribute__ ((packed));
   unsigned long  VideoCapabilities       __attribute__ ((packed));
   unsigned short VideoMinXScale          __attribute__ ((packed));
   unsigned short VideoMinYScale          __attribute__ ((packed));
   unsigned short VideoMaxXScale          __attribute__ ((packed));
   unsigned short VideoMaxYScale          __attribute__ ((packed));
   unsigned char  reserved[76]            __attribute__ ((packed));
} AF_MODE_INFO;



#define DC  struct AF_DRIVER *dc



typedef struct AF_DRIVER
{
   char           Signature[12]           __attribute__ ((packed));
   unsigned long  Version                 __attribute__ ((packed));
   unsigned long  DriverRev               __attribute__ ((packed));
   char           OemVendorName[80]       __attribute__ ((packed));
   char           OemCopyright[80]        __attribute__ ((packed));
   short          *AvailableModes         __attribute__ ((packed));
   unsigned long  TotalMemory             __attribute__ ((packed));
   unsigned long  Attributes              __attribute__ ((packed));
   unsigned long  BankSize                __attribute__ ((packed));
   unsigned long  BankedBasePtr           __attribute__ ((packed));
   unsigned long  LinearSize              __attribute__ ((packed));
   unsigned long  LinearBasePtr           __attribute__ ((packed));
   unsigned long  LinearGranularity       __attribute__ ((packed));
   unsigned short *IOPortsTable           __attribute__ ((packed));
   unsigned long  IOMemoryBase[4]         __attribute__ ((packed));
   unsigned long  IOMemoryLen[4]          __attribute__ ((packed));
   unsigned long  LinearStridePad         __attribute__ ((packed));
   unsigned short PCIVendorID             __attribute__ ((packed));
   unsigned short PCIDeviceID             __attribute__ ((packed));
   unsigned short PCISubSysVendorID       __attribute__ ((packed));
   unsigned short PCISubSysID             __attribute__ ((packed));
   unsigned long  Checksum                __attribute__ ((packed));
   unsigned long  res2[6]                 __attribute__ ((packed));
   void           *IOMemMaps[4]           __attribute__ ((packed));
   void           *BankedMem              __attribute__ ((packed));
   void           *LinearMem              __attribute__ ((packed));
   unsigned long  res3[5]                 __attribute__ ((packed));
   unsigned long  BufferEndX              __attribute__ ((packed));
   unsigned long  BufferEndY              __attribute__ ((packed));
   unsigned long  OriginOffset            __attribute__ ((packed));
   unsigned long  OffscreenOffset         __attribute__ ((packed));
   unsigned long  OffscreenStartY         __attribute__ ((packed));
   unsigned long  OffscreenEndY           __attribute__ ((packed));
   unsigned long  res4[10]                __attribute__ ((packed));
   unsigned long  SetBank32Len            __attribute__ ((packed));
   void           *SetBank32              __attribute__ ((packed));
   void           *Int86                  __attribute__ ((packed));
   void           *CallRealMode           __attribute__ ((packed));
   void           *InitDriver             __attribute__ ((packed));
   void           *af10Funcs[40]          __attribute__ ((packed));
   void           *PlugAndPlayInit        __attribute__ ((packed));
   void           *(*OemExt)(DC, unsigned long id);
   void           *SupplementalExt        __attribute__ ((packed));
   long           (*GetVideoModeInfo)(DC, short mode, AF_MODE_INFO *modeInfo);
} AF_DRIVER;



#undef DC



#define FAF_ID(a,b,c,d)    ((a<<24) | (b<<16) | (c<<8) | d)

#define FAFEXT_INIT     FAF_ID('I','N','I','T')
#define FAFEXT_MAGIC    FAF_ID('E','X', 0,  0)

#define FAFEXT_LIBC     FAF_ID('L','I','B','C')
#define FAFEXT_PMODE    FAF_ID('P','M','O','D')

#define FAFEXT_HWPTR    FAF_ID('H','P','T','R')

#define FAFEXT_CONFIG   FAF_ID('C','O','N','F')


typedef struct FAF_CONFIG_DATA
{
   unsigned long id;
   unsigned long value;
} FAF_CONFIG_DATA;



AF_DRIVER *af = NULL;

unsigned long af_memmap[4] = { 0, 0, 0, 0 };
unsigned long af_banked_mem = 0;
unsigned long af_linear_mem = 0;

extern void _af_int86(), _af_call_rm();



void af_exit()
{
   int c;

   for (c=0; c<4; c++)
      _remove_linear_mapping(&af_memmap[c]);

   _remove_linear_mapping(&af_banked_mem);
   _remove_linear_mapping(&af_linear_mem);

   if (af) {
      free(af);
      af = NULL;
   }
}



static int call_vbeaf_asm(void *proc)
{
   int ret;

   proc = (void *)((long)af + (long)proc);

   asm (
      "  call *%2 "

   : "=&a" (ret) 
   : "b" (af), 
     "rm" (proc) 
   : "memory" 
   );

   return ret;
}



int load_vbeaf_driver(char *filename)
{
   long size;
   PACKFILE *f;

   size = file_size(filename);
   if (size <= 0)
      return -1;

   f = pack_fopen(filename, F_READ);
   if (!f)
      return -1;

   af = malloc(size);

   if (pack_fread(af, size, f) != size) {
      free(af);
      af = NULL;
      return -1;
   }

   pack_fclose(f);

   return 0;
}



void initialise_freebeaf_extensions()
{
   typedef unsigned long (*EXT_INIT_FUNC)(AF_DRIVER *af, unsigned long id);
   EXT_INIT_FUNC ext_init;
   FAF_CONFIG_DATA *cfg;
   unsigned long magic;
   int v1, v2;

   if (!af->OemExt) {
      printf("no OemExt\n");
      return;
   }

   ext_init = (EXT_INIT_FUNC)((long)af + (long)af->OemExt);

   magic = ext_init(af, FAFEXT_INIT);

   v1 = (magic>>8)&0xFF;
   v2 = magic&0xFF;

   if (((magic&0xFFFF0000) != FAFEXT_MAGIC) || (!isdigit(v1)) || (!isdigit(v2))) {
      printf("bad magic number!\n");
      return;
   }

   printf("EX%c%c", v1, v2);

   if (af->OemExt(af, FAFEXT_LIBC))
      printf(", FAFEXT_LIBC");

   if (af->OemExt(af, FAFEXT_PMODE))
      printf(", FAFEXT_PMODE");

   if (af->OemExt(af, FAFEXT_HWPTR))
      printf(", FAFEXT_HWPTR");

   cfg = af->OemExt(af, FAFEXT_CONFIG);
   if (cfg)
      printf(", FAFEXT_CONFIG");

   printf("\n");

   if (cfg) {
      while (cfg->id) {
	 printf("Config variable:\t%c%c%c%c = 0x%08lX\n", 
		(char)(cfg->id>>24), (char)(cfg->id>>16), 
		(char)(cfg->id>>8), (char)(cfg->id), cfg->value);

	 cfg++;
      }
   }
}



static int initialise_vbeaf()
{
   int c;

   if (__djgpp_nearptr_enable() == 0)
      return -1;

   for (c=0; c<4; c++) {
      if (af->IOMemoryBase[c]) {
	 if (_create_linear_mapping(&af_memmap[c], af->IOMemoryBase[c], 
				    af->IOMemoryLen[c]) != 0)
	    return -1;

	 af->IOMemMaps[c] = (void *)(af_memmap[c]-__djgpp_base_address);
      }
   }

   if (af->BankedBasePtr) {
      if (_create_linear_mapping(&af_banked_mem, af->BankedBasePtr, 0x10000) != 0)
	 return -1;

      af->BankedMem = (void *)(af_banked_mem-__djgpp_base_address);
   }

   if (af->LinearBasePtr) {
      if (_create_linear_mapping(&af_linear_mem, af->LinearBasePtr, af->LinearSize*1024) != 0)
	 return -1;

      af->LinearMem  = (void *)(af_linear_mem-__djgpp_base_address);
   }

   af->Int86 = _af_int86;
   af->CallRealMode = _af_call_rm;

   return 0;
}



void print_af_attributes(unsigned long attrib)
{
   static char *flags[] =
   {
      "multibuf",
      "scroll",
      "banked",
      "linear",
      "accel",
      "dualbuf",
      "hwcursor",
      "8bitdac",
      "nonvga",
      "2scan",
      "interlace",
      "3buffer",
      "stereo",
      "rop2",
      "hwstsync",
      "evcstsync"
   };

   int first = TRUE;
   int c;

   printf("Attributes:\t\t");

   for (c=0; c<(int)(sizeof(flags)/sizeof(flags[0])); c++) {
      if (attrib & (1<<c)) {
	 if (first)
	    first = FALSE;
	 else
	    printf(", ");

	 printf(flags[c]);
      }
   }

   printf("\n");
}



int get_af_info()
{
   char filename[256];
   char *p;
   int c, attrib;

   p = get_config_string(NULL, "vbeaf_driver", NULL);
   if ((p) && (*p)) {
      strcpy(filename, p);
      if (*get_filename(filename) == 0) {
	 strcat(filename, "vbeaf.drv");
      }
      else {
	 if (file_exists(filename, FA_DIREC, &attrib)) {
	    if (attrib & FA_DIREC) {
	       put_backslash(filename);
	       strcat(filename, "vbeaf.drv");
	    }
	 }
      }
      if (load_vbeaf_driver(filename) == 0)
	 goto found_it;
   }

   strcpy(filename, "c:\\vbeaf.drv");
   if (load_vbeaf_driver(filename) == 0)
      goto found_it;

   p = getenv("VBEAF_PATH");
   if (p) {
      strcpy(filename, p);
      put_backslash(filename);
      strcat(filename, "vbeaf.drv");
      if (load_vbeaf_driver(filename) == 0)
	 goto found_it;
   }

   printf("Error: no VBE/AF driver found!\nYou should have a vbeaf.drv file in the root of your c: drive\n");
   return -1;

   found_it:

   printf("Driver file:\t\t%s\n", filename);

   if (strcmp(af->Signature, "VBEAF.DRV") != 0) {
      af_exit();
      printf("\nError: bad header ID in this driver file!\n");
      return -1;
   }

   if (af->Version < 0x200) {
      af_exit();
      printf("Error: obsolete driver version! (0x%lX)\n", af->Version);
      return -1;
   }

   printf("FreeBE/AF ext:\t\t");

   if (strstr(af->OemVendorName, "FreeBE"))
      initialise_freebeaf_extensions();
   else
      printf("not a FreeBE/AF driver\n");

   if (call_vbeaf_asm(af->PlugAndPlayInit) != 0) {
      af_exit();
      printf("\nError: VBE/AF Plug and Play initialisation failed!\n");
      return -1;
   }

   if (initialise_vbeaf() != 0) {
      af_exit();
      return -1;
   }

   if (call_vbeaf_asm(af->InitDriver) != 0) {
      af_exit();
      printf("\nError: VBE/AF device not present!\n");
      return -1;
   }

   printf("VBE/AF version:\t\t0x%lX\n", af->Version);
   printf("VendorName:\t\t%s\n", af->OemVendorName);
   printf("VendorCopyright:\t%s\n", af->OemCopyright);
   printf("TotalMemory:\t\t%ld\n", af->TotalMemory);

   print_af_attributes(af->Attributes);

   if (verbose) {
      printf("BankedAddr:\t\t0x%08lX\n", af->BankedBasePtr);
      printf("BankedSize:\t\t0x%08lX\n", af->BankSize*1024);
      printf("LinearAddr:\t\t0x%08lX\n", af->LinearBasePtr);
      printf("LinearSize:\t\t0x%08lX\n", af->LinearSize*1024);

      for (c=0; c<4; c++) {
	 printf("IOMemoryBase[%d]:\t0x%08lX\n", c, af->IOMemoryBase[c]);
	 printf("IOMemoryLen[%d]:\t\t0x%08lX\n", c, af->IOMemoryLen[c]);
      }

      printf("\n\n\n");
   }
   else
      printf("\n");

   return 0;
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
   AF_MODE_INFO mode_info;
   char color_desc[80];
   char buf[80];
   int c;

   if (!verbose) {
      sprintf(buf, "%X:", mode);
      printf("Mode 0x%-5s", buf);
   }
   else
      printf("Mode 0x%X:\n\n", mode);

   if (af->GetVideoModeInfo(af, mode, &mode_info) != 0) {
      printf("GetVideoModeInfo failed!\n");
      if (verbose)
	 printf("\n\n\n");
      return -1;
   }

   add_color(mode_info.RedMaskSize, mode_info.RedFieldPosition, 'R');
   add_color(mode_info.GreenMaskSize, mode_info.GreenFieldPosition, 'G');
   add_color(mode_info.BlueMaskSize, mode_info.BlueFieldPosition, 'B');
   add_color(mode_info.RsvdMaskSize, mode_info.RsvdFieldPosition, 'x');
   get_color_desc(color_desc);

   if (verbose) {
      print_af_attributes(mode_info.Attributes);

      if (color_desc[0])
	 printf("Format:\t\t\t%s\n", color_desc);

      printf("XResolution:\t\t%d\n", mode_info.XResolution);
      printf("YResolution:\t\t%d\n", mode_info.YResolution);
      printf("BitsPerPixel:\t\t%d\n", mode_info.BitsPerPixel);
      printf("BytesPerScanLine:\t%d\n", mode_info.BytesPerScanLine);
      printf("LinBytesPerScanLine:\t%d\n", mode_info.LinBytesPerScanLine);
      printf("MaxBytesPerScanLine:\t%d\n", mode_info.MaxBytesPerScanLine);
      printf("MaxScanLineWidth:\t%d\n", mode_info.MaxScanLineWidth);
      printf("MaxBuffers:\t\t%d\n", mode_info.MaxBuffers);
      printf("BnkMaxBuffers:\t\t%d\n", mode_info.BnkMaxBuffers);
      printf("LinMaxBuffers:\t\t%d\n", mode_info.LinMaxBuffers);
      printf("RedMaskSize:\t\t%d\n", mode_info.RedMaskSize);
      printf("RedFieldPos:\t\t%d\n", mode_info.RedFieldPosition);
      printf("GreenMaskSize:\t\t%d\n", mode_info.GreenMaskSize);
      printf("GreenFieldPos:\t\t%d\n", mode_info.GreenFieldPosition);
      printf("BlueMaskSize:\t\t%d\n", mode_info.BlueMaskSize);
      printf("BlueFieldPos:\t\t%d\n", mode_info.BlueFieldPosition);
      printf("RsvdMaskSize:\t\t%d\n", mode_info.RsvdMaskSize);
      printf("RsvdFieldPos:\t\t%d\n", mode_info.RsvdFieldPosition);
      printf("LinRedMaskSize:\t\t%d\n", mode_info.LinRedMaskSize);
      printf("LinRedFieldPos:\t\t%d\n", mode_info.LinRedFieldPosition);
      printf("LinGreenMaskSize:\t%d\n", mode_info.LinGreenMaskSize);
      printf("LinGreenFieldPos:\t%d\n", mode_info.LinGreenFieldPosition);
      printf("LinBlueMaskSize:\t%d\n", mode_info.LinBlueMaskSize);
      printf("LinBlueFieldPos:\t%d\n", mode_info.LinBlueFieldPosition);
      printf("LinRsvdMaskSize:\t%d\n", mode_info.LinRsvdMaskSize);
      printf("LinRsvdFieldPos:\t%d\n", mode_info.LinRsvdFieldPosition);
      printf("MaxPixelClock:\t\t%ld\n", mode_info.MaxPixelClock);

      printf("\n\n\n");
   }
   else {
      printf("%5dx%-6d", mode_info.XResolution, mode_info.YResolution);
      printf("%d bpp", mode_info.BitsPerPixel);
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

   printf("\nAllegro VBE/AF info utility " ALLEGRO_VERSION_STR);
   printf("\nBy Shawn Hargreaves, " ALLEGRO_DATE_STR "\n\n");

   if (get_af_info() != 0)
      return -1;

   for (c=0; af->AvailableModes[c] != -1; c++)
      get_mode_info(af->AvailableModes[c]);

   if (!verbose)
      printf("\n'afinfo -v' for a verbose listing\n\n");

   af_exit();

   return 0;
}


