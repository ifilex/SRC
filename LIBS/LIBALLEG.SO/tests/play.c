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
 *      Sound code test program for the Allegro library.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef DJGPP
#include <conio.h>
#include <go32.h>
#include <sys/farptr.h>
#endif

#include "allegro.h"


#ifdef __GNUC__
void usage() __attribute__ ((noreturn));
#endif


void usage()
{
   printf("\nSound code test program for Allegro " ALLEGRO_VERSION_STR);
   printf("\nBy Shawn Hargreaves, " ALLEGRO_DATE_STR "\n\n");
   printf("Usage: play [digital driver [midi driver]] files.(mid|voc|wav)\n");

   printf("\nDigital drivers:\n");
   printf("    '0'    (none)                  'SB'   (autodetect breed of SB)\n");
   printf("    'SB10' (SB 1.0                 'SB15' (SB 1.5)\n");
   printf("    'SB20' (SB 2.0                 'SBP'  (SB Pro)\n");
   printf("    'SB16' (SB 16)                 'ESS'  (ESS AudioDrive)\n");
   printf("    'ESC'  (Ensoniq Soundscape)\n");

   printf("\nMidi drivers:\n");
   printf("    '0'    (none)                  'OPL'  (Adlib, autodetect version)\n");
   printf("    'OPL2' (2-op FM synth)         'OPLX' (dual OPL2 chips in SB Pro-1)\n");
   printf("    'OPL3' (3-op FM synth)         'SB'   (raw SB MIDI interface)\n");
   printf("    'MPU'  (raw MPU-401 output)    'DIGI' (software wavetable)\n");
   printf("    'AWE'  (AWE32)\n");

   printf("\nIf you don't specify the card, Allegro will auto-detect (ie. guess :-)\n");

   exit(1);
}



void quiet_put(char *buf, int line)
{
   /* some SB cards produce loads of static if we sit there in a tight 
    * loop calling the BIOS text output routines, so this method is required.
    */

#ifdef DJGPP
   int c;
   long addr;

   _farsetsel(_dos_ds);
   addr = 0xb8000 + line * 160;

   for (c=0; buf[c]; c++) {
      _farnspokeb(addr, buf[c]);
      addr += 2;
   }

   while (c++ < 80) {
      _farnspokeb(addr, 0);
      addr += 2;
   }
#else
   printf("\r%-75s", buf);
   fflush(stdout);
#endif
}



int is_driver_id(char *s)
{
   int i;

   for (i=0; s[i]; i++) {
      if (!isalnum(s[i]))
	 return FALSE;

      if (i >= 4)
	 return FALSE;
   }

   return TRUE;
}



int get_driver_id(char *s)
{
   char tmp[4];
   char *endp;
   int val, i;

   val = strtol(s, &endp, 0);
   if (!*endp)
      return val;

   tmp[0] = tmp[1] = tmp[2] = tmp[3] = ' ';

   for (i=0; i<4; i++) {
      if (s[i])
	 tmp[i] = toupper(s[i]);
      else
	 break;
   }

   return AL_ID(tmp[0], tmp[1], tmp[2], tmp[3]);
}



int main(int argc, char *argv[])
{
   int digicard = -1;
   int midicard = -1;
   int k = 0;
   int vol = 255;
   int pan = 128;
   int freq = 1000;
   int paused = FALSE;
   char buf[80];
   void *item[9];
   int is_midi[9];
   int item_count = 0;
   int i, line;
   long old_midi_pos = -1;
   int doodle_note = -1;
   unsigned char midi_msg[3];
   int doodle_pitch[12] = { 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79 };

   if (argc < 2)
      usage();

   if ((argc > 2) && (is_driver_id(argv[1]))) {
      digicard = get_driver_id(argv[1]);
      if ((argc > 3) && (is_driver_id(argv[2]))) {
	 midicard = get_driver_id(argv[2]);
	 i = 3;
      }
      else
	 i = 2;
   }
   else
      i = 1;

   allegro_init();
   install_timer();
   install_keyboard();

   if (install_sound(digicard, midicard, argv[0]) != 0) {
      printf("\nError initialising sound system\n%s\n", allegro_error);
      allegro_exit();
      return 1;
   }

   printf("\nDigital sound driver: %s\n", digi_driver->name);
   printf("Description: %s\n", digi_driver->desc);
   printf("Voices: %d\n\n", digi_driver->voices);
   printf("Midi music driver: %s\n", midi_driver->name);
   printf("Description: %s\n", midi_driver->desc);
   printf("Voices: %d\n\n", midi_driver->voices);

   while ((i < argc) && (item_count < 9)) {
      if ((stricmp(get_extension(argv[i]), "voc") == 0) ||
	  (stricmp(get_extension(argv[i]), "wav") == 0)) {
	 item[item_count] = load_sample(argv[i]);
	 is_midi[item_count] = 0;
      }
      else if (stricmp(get_extension(argv[i]), "mid") == 0) {
	 item[item_count] = load_midi(argv[i]);
	 is_midi[item_count] = 1;
      }
      else {
	 printf("Unknown file type '%s'\n", argv[i]);
	 goto get_out;
      }

      if (!item[item_count]) {
	 printf("Error reading %s\n", argv[i]);
	 goto get_out;
      }

      printf("%d: %s\n", ++item_count, argv[i]);
      i++;
   }

   printf("\nPress a number 1-9 to trigger a sample or midi file\n");
   printf("v/V changes sfx volume, p/P changes sfx pan, and f/F changes sfx frequency\n");
   printf("space pauses/resumes MIDI playback, and the arrow keys seek through the tune\n");
   printf("Use the function keys to doodle a tune\n\n\n");

#ifdef DJGPP
   line = wherey() - 2;
#else
   line = 0;
#endif

   k = '1';      /* start sound automatically */

   do {
      switch (k & 0xFF) {

	 case 'v':
	    vol -= 8;
	    if (vol < 0)
	       vol = 0;
	    set_volume(-1, vol);
	    break;

	 case 'V':
	    vol += 8;
	    if (vol > 255)
	       vol = 255;
	    set_volume(-1, vol);
	    break;

	 case 'p':
	    pan -= 8;
	    if (pan < 0)
	       pan = 0;
	    break;

	 case 'P':
	    pan += 8;
	    if (pan > 255)
	       pan = 255;
	    break;

	 case 'f':
	    freq -= 8;
	    if (freq < 1)
	       freq = 1;
	    break;

	 case 'F':
	    freq += 8;
	    break;

	 case '0':
	    play_midi(NULL, FALSE);
	    paused = FALSE;
	    break;

	 case ' ':
	    if (paused) {
	       midi_resume();
	       paused = FALSE;
	    }
	    else {
	       midi_pause();
	       paused = TRUE;
	    }
	    break;

	 default:
	    if ((k >> 8) == KEY_LEFT) {
	       for (i=midi_pos-1; (midi_pos == old_midi_pos) && (midi_pos > 0); i--)
		  midi_seek(i);
	       paused = FALSE;
	    }
	    else if ((k >> 8) == KEY_RIGHT) {
	       for (i=midi_pos+1; (midi_pos == old_midi_pos) && (midi_pos > 0); i++)
		  midi_seek(i);
	       paused = FALSE;
	    }
	    if ((k >> 8) == KEY_UP) {
	       for (i=midi_pos-16; (midi_pos == old_midi_pos) && (midi_pos > 0); i--)
		  midi_seek(i);
	       paused = FALSE;
	    }
	    else if ((k >> 8) == KEY_DOWN) {
	       for (i=midi_pos+16; (midi_pos == old_midi_pos) && (midi_pos > 0); i++)
		  midi_seek(i);
	       paused = FALSE;
	    }
	    else if (((k & 0xFF) >= '1') && ((k & 0xFF) < '1'+item_count)) {
	       k = (k & 0xFF) - '1';
	       if (is_midi[k]) {
		  play_midi((MIDI *)item[k], FALSE);
		  paused = FALSE;
	       }
	       else
		  play_sample((SAMPLE *)item[k], vol, pan, freq, FALSE);
	    }
	    else {
	       k >>= 8;
	       if (((k >= KEY_F1) && (k <= KEY_F10)) ||
		   ((k >= KEY_F11) && (k <= KEY_F12))) {
		  if (doodle_note >= 0) {
		     midi_msg[0] = 0x80;
		     midi_msg[1] = doodle_pitch[doodle_note];
		     midi_msg[2] = 0;
		     midi_out(midi_msg, 3);
		  }

		  if ((k >= KEY_F1) && (k <= KEY_F10))
		     doodle_note = k - KEY_F1;
		  else
		     doodle_note = 10 + k - KEY_F11;

		  midi_msg[0] = 0x90;
		  midi_msg[1] = doodle_pitch[doodle_note];
		  midi_msg[2] = 127;
		  midi_out(midi_msg, 3);
	       }
	    }
	    break;
      }

      old_midi_pos = midi_pos;

      sprintf(buf, "midi pos: %ld   vol: %d   pan: %d   freq: %d",
						midi_pos, vol, pan, freq);
      quiet_put(buf, line);

      do {
      } while ((!keypressed()) && (midi_pos == old_midi_pos));

      if (keypressed())
	 k = readkey();
      else
	 k = 0;

   } while ((k & 0xFF) != 27);

   printf("\n");

   get_out:

   for (i=0; i<item_count; i++)
      if (is_midi[i])
	 destroy_midi((MIDI *)item[i]);
      else
	 destroy_sample((SAMPLE *)item[i]);

   return 0;
}

