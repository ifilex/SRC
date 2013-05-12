/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program demonstrates how to play samples.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


int main(int argc, char *argv[])
{
   SAMPLE *the_sample;
   int pan = 128;
   int pitch = 1000;

   if (argc != 2) {
      printf("Usage: 'ex17 filename.[wav|voc]'\n");
      return 1;
   }

   allegro_init();
   install_keyboard(); 
   install_timer();

   /* install a digital sound driver */
   if (install_sound(DIGI_AUTODETECT, MIDI_NONE, argv[0]) != 0) {
      printf("Error initialising sound system\n%s\n", allegro_error);
      return 1;
   }

   /* read in the WAV file */
   the_sample = load_sample(argv[1]);
   if (!the_sample) {
      printf("Error reading WAV file '%s'\n", argv[1]);
      return 1;
   }

   printf("Digital sound driver: %s\n", digi_driver->name);
   printf("Playing %s\n", argv[1]);
   printf("Use the arrow keys to adjust it\n");

   /* start up the sample */
   play_sample(the_sample, 255, pan, pitch, TRUE);

   do {
      /* alter the pan position? */
      if ((key[KEY_LEFT]) && (pan > 0))
	 pan--;
      else if ((key[KEY_RIGHT]) && (pan < 255))
	 pan++;

      /* alter the pitch? */
      if ((key[KEY_UP]) && (pitch < 16384))
	 pitch = ((pitch * 513) / 512) + 1; 
      else if ((key[KEY_DOWN]) && (pitch > 64))
	 pitch = ((pitch * 511) / 512) - 1; 

      /* adjust the sample */
      adjust_sample(the_sample, 255, pan, pitch, TRUE);

      /* delay a bit */
      rest(2);

   } while ((!key[KEY_ESC]) && (!key[KEY_SPACE]));

   /* destroy the sample */
   destroy_sample(the_sample);

   return 0;
}
