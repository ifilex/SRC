/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    Plays MIDI files in the background. A dodgy trick, and not
 *    really useful for anything, but I think it is pretty cool!
 */


#include <stdlib.h>
#include <stdio.h>

#ifdef DJGPP
#include <dos.h>
#include <sys/exceptn.h>
#endif

#include "allegro.h"


int main(int argc, char *argv[])
{
   MIDI *the_music;

   if (argc != 2) {
      printf("Usage: 'ex33 filename.mid'\n");
      return 1;
   }

#ifdef DJGPP
   __djgpp_set_ctrl_c(0);
   setcbrk(0);
#endif

   allegro_init();
   install_timer();

   if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, argv[0]) != 0) {
      printf("Error initialising sound system\n%s\n", allegro_error);
      return 1;
   }

   the_music = load_midi(argv[1]);
   if (!the_music) {
      printf("Error reading MIDI file '%s'\n", argv[1]);
      return 1;
   }

   printf("\nMusic driver: %s\n", midi_driver->name);
   printf("Playing %s\n\n", argv[1]);

   if (windows_version) {
      printf("I seem to be running under a multitasking environment. This means that I\n");
      printf("may not work properly, and even if I do, the sound will probably cut out\n");
      printf("whenever you exit from a child program. Please run me under DOS instead!\n\n");
   }

   printf("You must not run any programs that trap hardware interrupts\n");
   printf("or access the soundcard from this command prompt!\n\n");
   printf("Type \"exit\" to quit.\n\n");
   printf("----------------------------------------------------------------\n\n");

   play_midi(the_music, TRUE);

   system(NULL);

   printf("\n----------------------------------------------------------------\n\n");
   printf("Shutting down the music player...\n");
   printf("Goodbye!\n\n");

   destroy_midi(the_music);

   return 0;
}

