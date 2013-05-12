/*----------------------------------------------------------------------------*/
/*                         Allegro Library example file.                      */
/*                                                                            */
/* This example demonstrates how to use date files, various sprite drawing    */
/* routines and flicker-free animation.                                       */
/*----------------------------------------------------------------------------*/
/* by Grzegorz "LUD0" Ludorowski (pajonk@ajax.umcs.lublin.pl)                 */
/*                               (http://ajax.umcs.lublin.pl/~pajonk)         */
/* Sprite by me too.                                                          */
/*----------------------------------------------------------------------------*/

/*
 *  A short explanation for beginners.
 *  Why did I do animate () routine in that way?
 *  As you probably know, VIDEO RAM is much slower than "normal" RAM, so
 *  it's advisable to reduce VRAM blits to a minimum.
 *  So, drawing sprite on the screen (I mean in VRAM) and then clearing
 *  a background for him is not very fast. I've used a different method which
 *  is much faster, but require a bit more memory.
 *  I clear a buffer (it's a normal BITMAP), then I draw sprite to it, and
 *  after all I blit only one time this buffer to the screen. So, I'm using a
 *  single VRAM blit instead of blitting/clearing background and drawing
 *  a sprite on it. It's a good method even when I have to restore background.
 *  And of course, it completely remove flickering effect.
 *  When one uses a big (ie. 800x600 background) and draws something on it,
 *  it's wise to use a copy of background somewhere in memory and restore
 *  background using this "virtual background". When blitting from VRAM in SVGA
 *  modes, it's probably, that drawing routines have to switch banks on video
 *  card. I think, I don't have to remind how slow is it.
*/


#include <stdlib.h>
#include <stdio.h>

#include "running.h"
#include "allegro.h"

/* Pointer to data file */
DATAFILE *running_data;
/* w-screen width, h-screen height, card-card type (defined in allegro.h) */
int card, w, h;
/* Place to store set_gfx_mode () output. -1 means error */
int err_vmode;
/* current sprite frame number */
int frame_number;
/* Pointer to a sprite buffer, where sprite will be drawn */
BITMAP *sprite_buffer;
/* A boolean - if true, skip to next part */
int next;
/* Used in rotate_sprite routine */
int angle;
/* Used in file_exists routine   */
int *file_attr;

void animate ()
{
   /* Waits for vertical retrace interrupt */
   vsync ();   vsync ();
   /* Blits sprite buffer to screen. */
   blit (sprite_buffer, screen, 0,0,120,80,85,85);
   /* Clears sprite buffer with color 0 */
   clear (sprite_buffer);
   next = FALSE;
   /* If SPACE key pressed set a next flag */
   if ( key[KEY_SPACE] ) next = TRUE;
   /* Increase frame number, or if it's equal 9 (last frame) set it to 0 */
   if ( frame_number == 9 ) { frame_number = 0; } else { frame_number++; }
}

/* Begin of main routine */
int main (int argc, char *argv[])
{
   char datafile_name[256];
   replace_filename(datafile_name, argv[0], "running.dat", sizeof(datafile_name));

   allegro_init();              /* Initialise all Allegro stuff       */
   install_keyboard();          /* Install keyboard interrupt handler */
   install_mouse();             /* Install mouse handler              */
   /* Removes everything from keyboard's buffer */
   clear_keybuf();
   card = GFX_VGA;
   w = 320;
   h = 200;
   angle = 0;                   /* Set angle to 0 */
   frame_number = 0;            /* Begin with the first sprite frame */
   /* Sets 320x200x256 mode */
   err_vmode = set_gfx_mode(card, w, h, 0, 0);
   /* Checks if any error occured during screen mode initialization   */
   if ( err_vmode != 0 )
   {
    printf("Error setting graphics mode\n%s\n\n", allegro_error);
   /* If error - shut down Allegro          */
    allegro_exit ();
    return 1;
   }
   /* Loads datafile and sets user pallete saved in datafile */
   running_data = load_datafile (datafile_name);
   if (!running_data)
   {
    allegro_exit();
    printf("Error loading %s!\n", datafile_name);
    return 1;
   }
   set_pallete(running_data[PALLETE_001].dat);
   /* Create and clear a bitmap for sprite buffering */
   sprite_buffer = create_bitmap (85,85);
   clear (sprite_buffer);
   /* Write current sprite drawing method */
   textout (screen, font, "Press space bar for next part..",40,10,1);
   textout (screen, font, "Using draw_sprite",1,190,15);
	do
	{
	    draw_sprite (sprite_buffer, running_data[frame_number].dat,0,0);
	    animate ();     /* Draw sprite to screen with buffering */
	}
	while ( next==FALSE );
   /* Clears keyboard buffer and place for next sign */
   clear_keybuf (); rectfill (screen, 0,190,320,200,0);
   textout (screen, font, "Using draw_sprite_h_flip",1,190,15);
	do
	{
	    draw_sprite_h_flip (sprite_buffer, running_data[frame_number].dat,0,0);
	    animate ();
	}
	while ( next==FALSE );
   clear_keybuf (); rectfill (screen, 0,190,320,200,0);
   textout (screen, font, "Using draw_sprite_v_flip",1,190,15);
	do
	{
	    draw_sprite_v_flip (sprite_buffer, running_data[frame_number].dat,0,0);
	    animate ();
	}
	while ( next==FALSE );
   clear_keybuf (); rectfill (screen, 0,190,320,200,0);
   textout (screen, font, "Using draw_sprite_vh_flip",1,190,15);
	do
	{   draw_sprite_vh_flip (sprite_buffer, running_data[frame_number].dat,0,0);
	    animate ();
	}
	while ( next==FALSE );
   clear_keybuf (); rectfill (screen, 0,190,320,200,0);
   textout (screen, font, "Now with rotating - rotate_sprite",1,190,15);
	do
	  /* Last argument to rotate_sprite () is a fixed point type,  */
	  /*  so I had to use itofix (); routine ( integer to fixed ). */
	{   rotate_sprite (sprite_buffer, running_data[frame_number].dat,0,0,itofix (angle));
	    animate ();
	    if ( angle >= 255 ) {angle = 1;} {angle+=4;}
	}
	while ( next==FALSE );
   /* Unloads datafile and frees memory */
   unload_datafile (running_data);
   /* Shuts down Allegro     */
   allegro_exit();
   /* Exit program           */
   return 0;
}
