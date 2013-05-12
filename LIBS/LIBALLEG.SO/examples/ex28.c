/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program shows how to use the audio stream functions to transfer
 *    large blocks of sample data to the soundcard.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


#define BUFFER_SIZE  4096


int main()
{
   AUDIOSTREAM *stream;
   int updates = 0;
   int pitch = 0;
   int val = 0;
   int i;

   allegro_init();
   install_keyboard(); 
   install_timer();

   /* install a digital sound driver */
   if (install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL) != 0) {
      printf("Error initialising sound system\n%s\n", allegro_error);
      return 1;
   }

   /* create an audio stream */
   stream = play_audio_stream(BUFFER_SIZE, 8, FALSE, 22050, 255, 128);
   if (!stream) {
      printf("Error creating audio stream!\n");
      return 1;
   }

   printf("\nAudio stream is now playing...\n\n");

   while (!keypressed()) {
      /* does the stream need any more data yet? */
      unsigned char *p = get_audio_stream_buffer(stream);

      if (p) {
	 /* if so, generate a bit more of our waveform... */
	 printf("update #%d\n", updates++);

	 for (i=0; i<BUFFER_SIZE; i++) {
	    /* this is just a sawtooth wave that gradually increases in 
	     * pitch. Obviously you would want to do something a bit more
	     * interesting here, for example you could fread() the next
	     * buffer of data in from a disk file...
	     */
	    p[i] = (val >> 16) & 0xFF;
	    val += pitch;
	    pitch++;
	    if (pitch > 0x40000)
	       pitch = 0x10000;
	 }

	 free_audio_stream_buffer(stream);
      }
   }

   stop_audio_stream(stream);

   return 0;
}
