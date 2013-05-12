/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program demonstrates how to play MIDI files.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


int main(int argc, char *argv[])
{
   MIDI *the_music;

   if (argc != 2) {
      printf("Usage: 'ex16 filename.mid'\n");
      return 1;
   }

   allegro_init();
   install_keyboard(); 
   install_timer();

   /* install a MIDI sound driver */
   if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, argv[0]) != 0) {
      printf("Error initialising sound system\n%s\n", allegro_error);
      return 1;
   }

   /* read in the MIDI file */
   the_music = load_midi(argv[1]);
   if (!the_music) {
      printf("Error reading MIDI file '%s'\n", argv[1]);
      return 1;
   }

   printf("Midi driver: %s\n", midi_driver->name);
   printf("Playing %s\n", argv[1]);

   /* start up the MIDI file */
   play_midi(the_music, TRUE);

   /* wait for a keypress */
   readkey();

   /* destroy the MIDI file */
   destroy_midi(the_music);

   return 0;
}
