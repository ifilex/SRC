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
 *      Sound setup utility for the Allegro library.
 *
 *      By Shawn Hargreaves, based on code by David Calvin.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "internal.h"
#include "setupdat.h"



/* these can be customised to suit your program... */
#define TITLE           "Allegro Setup " ALLEGRO_VERSION_STR
#define CFG_FILE        "allegro.cfg"
#define KEYBOARD_FILE   "keyboard.dat"
#define LANGUAGE_FILE   "language.dat"



/* we only need the VGA 13h, 256 color drivers */
DECLARE_GFX_DRIVER_LIST(GFX_DRIVER_VGA);
DECLARE_COLOR_DEPTH_LIST(COLOR_DEPTH_8);



/* info about a hardware driver */
typedef struct SOUNDCARD
{
   int id;
   char *name;
   char **param;
   char *desc;
   int present;
} SOUNDCARD;


SOUNDCARD *soundcard;



/* list which parameters are used by each driver */
char *digi_auto_param[]    = { "11", "digi_volume", NULL };
char *digi_sbm_param[]     = { "sb_port", "sb_dma", "sb_irq", "digi_volume", "digi_voices", "sb_freq", NULL };
char *digi_sbs_param[]     = { "sb_port", "sb_dma", "sb_irq", "flip_pan", "quality", "sb_freq", "digi_volume", "digi_voices", NULL };
char *digi_esc_param[]     = { "02", "flip_pan", "quality", "sb_freq", "digi_volume", "digi_voices", NULL };

char *midi_auto_param[]    = { "11", "midi_volume", NULL };
char *midi_adlib_param[]   = { "22", "fm_port", "midi_volume", "", "ibk_file", "ibk_drum_file", NULL };
char *midi_sb_param[]      = { "21", "sb_port", "midi_volume", NULL };
char *midi_mpu_param[]     = { "22", "mpu_port", "mpu_irq", "", "midi_volume", NULL };
char *midi_digmid_param[]  = { "22", "midi_voices", "midi_volume", "", "patches", NULL };
char *midi_awe_param[]     = { "21", "midi_voices", "midi_volume", NULL };



/* list of digital sound drivers */
SOUNDCARD digi_cards[] =
{
   /* id                name                    parameter list       desc */
   { DIGI_AUTODETECT,   "Autodetect",           digi_auto_param,     "Attempt to autodetect the digital sound hardware",   0 },
   { DIGI_SB,           "Generic SB",           digi_sbs_param,      "SoundBlaster: autodetect the model",                 0 },
   { DIGI_SB10,         "SoundBlaster 1.0",     digi_sbm_param,      "SB v1.0, 8 bit mono using single-shot dma",          0 },
   { DIGI_SB15,         "SoundBlaster 1.5",     digi_sbm_param,      "SB v1.5, 8 bit mono using single shot dma",          0 },
   { DIGI_SB20,         "SoundBlaster 2.0",     digi_sbm_param,      "SB v2.0, 8 bit mono using auto-initialised dma",     0 },
   { DIGI_SBPRO,        "SoundBlaster Pro",     digi_sbs_param,      "SB Pro, 8 bit stereo DAC",                           0 },
   { DIGI_SB16,         "SoundBlaster 16",      digi_sbs_param,      "SB16 or AWE32, 16 bit stereo DAC",                   0 },
   { DIGI_AUDIODRIVE,   "ESS AudioDrive",       digi_sbs_param,      "ESS AudioDrive (16 bit stereo DAC)",                 0 },
   { DIGI_SOUNDSCAPE,   "Ensoniq Soundscape",   digi_esc_param,      "Ensoniq Soundscape (16 bit stereo DAC)",             0 },
   { DIGI_NONE,         "No Sound",             NULL,                "The Sound of Silence...",                            0 }
};



/* list of MIDI sound drivers */
SOUNDCARD midi_cards[] =
{
   /* id                name                    parameter list       desc */
   { MIDI_AUTODETECT,   "Autodetect",           midi_auto_param,     "Attempt to autodetect the MIDI hardware",                        0 },
   { MIDI_ADLIB,        "Generic Adlib",        midi_adlib_param,    "OPL FM synth: autodetect the model",                             0 },
   { MIDI_OPL2,         "Adlib (OPL2)",         midi_adlib_param,    "(mono) OPL2 FM synth (used in Adlib and standard SB cards)",     0 },
   { MIDI_2XOPL2,       "Adlib (dual OPL2)",    midi_adlib_param,    "(stereo) Two OPL2 FM synths (early SB Pro cards)",               0 },
   { MIDI_OPL3,         "Adlib (OPL3)",         midi_adlib_param,    "(stereo) OPL3 FM synth (Adlib Gold, later SB Pro boards, SB16)", 0 },
   { MIDI_AWE32,        "AWE32",                midi_awe_param,      "SoundBlaster AWE32 (EMU8000 synth chip)",                        0 },
   { MIDI_SB_OUT,       "SB MIDI-OUT",          midi_sb_param,       "Raw SB MIDI output to an external synth module",                 0 },
   { MIDI_MPU,          "MPU-401",              midi_mpu_param,      "Raw MPU MIDI output to an external synth module",                0 },
   { MIDI_DIGMID,       "Digital MIDI",         midi_digmid_param,   "Software wavetable synthesis using the digital sound hardware",  0 },
   { MIDI_NONE,         "No Sound",             NULL,                "The Sound of Silence...",                                        0 }
};



/* info about a joystick driver */
typedef struct HAPPINESS
{
   int id;
   char *name;
} HAPPINESS;



/* list of joystick drivers */
HAPPINESS joystick_list[] =
{
   /* id                      name */
   { JOY_TYPE_NONE,           "None" },
   { JOY_TYPE_STANDARD,       "Standard" },
   { JOY_TYPE_2PADS,          "Dual Joysticks" },
   { JOY_TYPE_4BUTTON,        "4-Button" },
   { JOY_TYPE_6BUTTON,        "6-Button" },
   { JOY_TYPE_8BUTTON,        "8-Button" },
   { JOY_TYPE_FSPRO,          "Flightstick Pro" },
   { JOY_TYPE_WINGEX,         "Wingman Extreme" },
   { JOY_TYPE_SIDEWINDER,     "Sidewinder" },
   { JOY_TYPE_GAMEPAD_PRO,    "Gamepad Pro" },
   { JOY_TYPE_SNESPAD_LPT1,   "SNESpad-LPT1" },
   { JOY_TYPE_SNESPAD_LPT2,   "SNESpad-LPT2" },
   { JOY_TYPE_SNESPAD_LPT3,   "SNESpad-LPT3" },
   { JOY_TYPE_WINGWARRIOR,    "Wingman Warrior" }
};



/* different types of parameter */
typedef enum PARAM_TYPE
{
   param_none,
   param_int,
   param_hex,
   param_id,
   param_bool,
   param_file,
   param_list
} PARAM_TYPE;



/* info about a soundcard parameter */
typedef struct PARAMETER
{
   char *name;
   PARAM_TYPE type;
   char value[80];
   char *def;
   int *detect;
   char *label;
   char *e1, *e2;
   char *desc;
} PARAMETER;



/* list of soundcard parameters */
PARAMETER parameters[] =
{
   /* name              type           value    default     detect            label       extra1      extra2   desc */
   { "digi_card",       param_id,      "",      "-1",       &digi_card,       "",         NULL,       NULL,    "" },
   { "midi_card",       param_id,      "",      "-1",       &midi_card,       "",         NULL,       NULL,    "" },
   { "digi_volume",     param_int,     "",      "-1",       NULL,             "Vol:",     NULL,       NULL,    "Digital sound volume (0 to 255)" },
   { "midi_volume",     param_int,     "",      "-1",       NULL,             "Vol:",     NULL,       NULL,    "MIDI music volume (0 to 255)" },
   { "digi_voices",     param_int,     "",      "-1",       NULL,             "Chan:",    NULL,       NULL,    "Number of channels reserved for playing digital sounds (higher values increase polyphony but degrade speed and quality)" },
   { "midi_voices",     param_int,     "",      "-1",       NULL,             "Chan:",    NULL,       NULL,    "Number of channels reserved for playing MIDI music (higher values increase polyphony but degrade speed and quality)" },
   { "flip_pan",        param_bool,    "",      "0",        NULL,             "Pan:",     NULL,       NULL,    "Reverses the left/right stereo placement" },
   { "quality",         param_int,     "",      "0",        &_sound_hq,       "Qual:",    NULL,       NULL,    "Sample mixing quality (0 = fast mixing of 8 bit data into 16 bit buffers, 1 = true 16 bit mixing, 2 = interpolated 16 bit mixing)" },
   { "sb_port",         param_hex,     "",      "-1",       &_sb_port,        "Port:",    NULL,       NULL,    "Port address (usually 220)" },
   { "sb_dma",          param_int,     "",      "-1",       &_sb_dma,         "DMA:",     NULL,       NULL,    "DMA channel (usually 1)" },
   { "sb_irq",          param_int,     "",      "-1",       &_sb_irq,         "IRQ:",     NULL,       NULL,    "IRQ number (usually 7)" },
   { "sb_freq",         param_list,    "",      "-1",       &_sb_freq,        "Freq:",    NULL,       NULL,    "Sample mixing frequency (higher values sound better but require more CPU processing time)" },
   { "fm_port",         param_hex,     "",      "-1",       &_fm_port,        "Port:",    NULL,       NULL,    "Port address (usually 388)" },
   { "mpu_port",        param_hex,     "",      "-1",       &_mpu_port,       "Port:",    NULL,       NULL,    "Port address (usually 330)" },
   { "mpu_irq",         param_int,     "",      "-1",       &_mpu_irq,        "IRQ:",     NULL,       NULL,    "IRQ number (usually 7)" },
   { "ibk_file",        param_file,    "",      "",         NULL,             "IBK:",     "IBK",      NULL,    "Custom .IBK instrument patch set" },
   { "ibk_drum_file",   param_file,    "",      "",         NULL,             "drumIBK:", "IBK",      NULL,    "Custom .IBK percussion patch set" },
   { "patches",         param_file,    "",      "",         NULL,             "Patches:", "CFG;DAT",  NULL,    "MIDI patch set (GUS format default.cfg or Allegro format patches.dat)" },
   { NULL,              param_none,    "",      "",         NULL,             NULL,       NULL,       NULL,    NULL } 
};



/* in some places we need to double-buffer the display... */
BITMAP *buffer;



/* dialogs do fancy stuff as they slide on and off the screen */
typedef enum DIALOG_STATE
{
   state_start,
   state_slideon,
   state_active,
   state_slideoff,
   state_exit,
   state_chain,
   state_redraw
} DIALOG_STATE;



/* info about an active dialog */
typedef struct ACTIVE_DIALOG
{
   DIALOG_STATE state;
   int time;
   DIALOG *dialog;
   DIALOG_PLAYER *player;
   BITMAP *buffer;
   DIALOG_STATE (*handler)(int c);
} ACTIVE_DIALOG;



/* list of active dialogs */
ACTIVE_DIALOG dialogs[4];

int dialog_count = 0;



/* scrolly text message at the base of the screen */
volatile int scroller_time = 0;
char scroller_msg[42];
int scroller_pos = 0;
int scroller_alpha = 256;
char *scroller_string = "";
char *wanted_scroller = "";
int scroller_string_pos = 0;



/* timer interrupt handler */
void inc_scroller_time()
{
   scroller_time++;
}

END_OF_FUNCTION(inc_scroller_time);



/* dialog procedure for animating the scroller text */
int scroller_proc(int msg, DIALOG *d, int c)
{
   int redraw = FALSE;
   int a, i, x;

   if (msg == MSG_IDLE) {
      while (scroller_time > 0) {
	 scroller_pos--;
	 if (scroller_pos <= -8) {
	    scroller_pos = 0;
	    for (i=0; i<(int)sizeof(scroller_msg)-1; i++)
	       scroller_msg[i] = scroller_msg[i+1];
	    if (scroller_string[scroller_string_pos])
	       scroller_msg[i] = scroller_string[scroller_string_pos++];
	    else 
	       scroller_msg[i] = ' ';
	    if (wanted_scroller != scroller_string) {
	       scroller_alpha -= MIN(32, scroller_alpha);
	       if (scroller_alpha <= 0) {
		  memset(scroller_msg, ' ', sizeof(scroller_msg));
		  scroller_string = wanted_scroller;
		  scroller_string_pos = 0;
		  for (x=0; x<4; x++) {
		     if (scroller_string[scroller_string_pos]) {
			for (i=0; i<(int)sizeof(scroller_msg)-1; i++)
			   scroller_msg[i] = scroller_msg[i+1];
			scroller_msg[i] = scroller_string[scroller_string_pos];
			scroller_string_pos++;
		     }
		  }
		  scroller_alpha = 256;
	       }
	    }
	    else
	       scroller_alpha += MIN(16, 256-scroller_alpha);
	 }
	 redraw = TRUE;
	 scroller_time--;
      }
   }
   else if (msg == MSG_RADIO) {
      memset(scroller_msg, ' ', sizeof(scroller_msg));
      scroller_string = wanted_scroller;
      scroller_string_pos = strlen(scroller_string);
      scroller_alpha = 256;
      redraw = TRUE;
   }

   if (redraw) {
      freeze_mouse_flag = TRUE;
      text_mode(0);

      for (i=0; i<(int)sizeof(scroller_msg); i++) {
	 x = i*8+scroller_pos;
	 a = 16 + MID(0, 15-ABS(SCREEN_W/2-x)/10, 15) * scroller_alpha/256;
	 textprintf(screen, font, x, SCREEN_H-16, a, "%c", scroller_msg[i]);
      }

      freeze_mouse_flag = FALSE;
   }

   return D_O_K;
}



/* helper for drawing a dialog onto a memory bitmap */
void draw_dialog(ACTIVE_DIALOG *d)
{
   BITMAP *oldscreen = screen;
   int nowhere;

   if (d->player->focus_obj >= 0) {
      SEND_MESSAGE(d->dialog+d->player->focus_obj, MSG_LOSTFOCUS, 0);
      d->dialog[d->player->focus_obj].flags &= ~D_GOTFOCUS;
      d->player->focus_obj = -1;
   }

   if (d->player->mouse_obj >= 0) {
      SEND_MESSAGE(d->dialog+d->player->mouse_obj, MSG_LOSTMOUSE, 0);
      d->dialog[d->player->mouse_obj].flags &= ~D_GOTMOUSE;
      d->player->mouse_obj = -1;
   }

   d->player->res &= ~D_WANTFOCUS;

   clear(d->buffer);
   screen = d->buffer; 
   dialog_message(d->dialog, MSG_DRAW, 0, &nowhere);
   screen = oldscreen;
}



/* start up another dialog */
void activate_dialog(DIALOG *dlg, DIALOG_STATE (*handler)(int c), int chain)
{
   ACTIVE_DIALOG *d = &dialogs[dialog_count];

   d->state = state_start;
   d->time = retrace_count;
   d->dialog = dlg;
   d->player = init_dialog(dlg, -1);
   d->buffer = create_bitmap(SCREEN_W, SCREEN_H);
   d->handler = handler;

   draw_dialog(d);

   if (dialog_count > 0) {
      draw_dialog(&dialogs[dialog_count-1]);
      dialogs[dialog_count-1].state = (chain ? state_chain : state_slideoff);
      dialogs[dialog_count-1].time = retrace_count;
   }
}



/* main dialog update routine */
int update()
{
   BITMAP *oldscreen = screen;
   ACTIVE_DIALOG *d;
   DIALOG_STATE state;
   PALETTE pal;
   int pos, ppos, pppos;
   int ret;

   if (dialog_count <= 0)
      return FALSE;

   d = &dialogs[dialog_count-1];

   if (d->state == state_active) {
      /* process the dialog */
      if (_mouse_screen != screen)
	 show_mouse(screen);

      ret = update_dialog(d->player);

      if (!ret) {
	 if (d->handler)
	    state = d->handler(d->player->obj);
	 else
	    state = state_exit;

	 if (state == state_exit) {
	    /* exit this dialog */
	    draw_dialog(d);
	    d->state = state_exit;
	    d->time = retrace_count;
	 }
	 else if (state == state_redraw) {
	    /* redraw the dialog */
	    d->player->res |= D_REDRAW;
	 }
      }
      else {
	 pos = find_dialog_focus(d->dialog);
	 if ((pos >= 0) && (d->dialog[pos].dp3))
	    wanted_scroller = d->dialog[pos].dp3;
      }
   }
   else {
      /* sliding on or off */
      show_mouse(NULL);

      blit(&background, buffer, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

      text_mode(0);
      textout_centre(buffer, font, TITLE, SCREEN_W/2, 0, -1);

      wanted_scroller = "";
      screen = buffer;
      scroller_proc(MSG_IDLE, NULL, 0);
      screen = oldscreen;

      pos = retrace_count - d->time;

      if ((dialog_count == 1) && (d->state == state_start))
	 pos *= 64;
      else if ((dialog_count == 1) && (d->state == state_exit))
	 pos *= 48;
      else if (d->state == state_start)
	 pos *= 96;
      else
	 pos *= 128;

      pos = MID(0, 4096-pos, 4096);
      pppos = (4096 - pos * pos / 4096);
      pos = pppos / 16;

      /* draw the slide effect */
      switch (d->state) {

	 case state_start:
	    ppos = pos;
	    stretch_sprite(buffer, d->buffer, 0, 0, SCREEN_W+1024-pppos/4, SCREEN_H+1024-pppos/4);
	    break;

	 case state_slideon:
	    ppos = pos;
	    stretch_sprite(buffer, d->buffer, 0, 0, SCREEN_W*ppos/256, SCREEN_H*ppos/256);
	    break;

	 case state_slideoff:
	 case state_chain:
	    ppos = 256 - pos;
	    stretch_sprite(buffer, d->buffer, SCREEN_W/2-SCREEN_W*ppos/512, SCREEN_H/2-SCREEN_H*ppos/512, SCREEN_W*ppos/256, SCREEN_H*ppos/256);
	    break;

	 case state_exit:
	    ppos = 256 - pos;
	    stretch_sprite(buffer, d->buffer, SCREEN_W-SCREEN_W*ppos/256, SCREEN_H-SCREEN_H*ppos/256, SCREEN_W*ppos/256, SCREEN_H*ppos/256);
	    break;

	 default:
	    ppos = 0;
	    break;
      }

      if ((dialog_count == 1) && (d->state != state_slideon) && (d->state != state_slideoff) && (d->state != state_chain)) {
	 fade_interpolate(black_palette, setup_pal, pal, ppos/4, 0, 255);
	 set_palette(pal);
      }
      else
	 vsync();

      blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

      if (pos >= 256) {
	 /* finished the slide */
	 switch (d->state) {

	    case state_start:
	    case state_slideon:
	       /* become active */
	       d->state = state_active;
	       d->player->res |= D_REDRAW;
	       break;

	    case state_slideoff:
	       /* activate next dialog, and then get ready to slide back on */
	       dialogs[dialog_count].time = retrace_count;
	       dialog_count++;
	       d->state = state_slideon;
	       break;

	    case state_exit:
	       /* time to die! */
	       shutdown_dialog(d->player);
	       destroy_bitmap(d->buffer);
	       dialog_count--;
	       if (dialog_count > 0)
		  dialogs[dialog_count-1].time = retrace_count;
	       break;

	    case state_chain:
	       /* kill myself, then activate the next dialog */
	       shutdown_dialog(d->player);
	       destroy_bitmap(d->buffer);
	       dialogs[dialog_count].time = retrace_count;
	       dialogs[dialog_count-1] = dialogs[dialog_count];
	       break;

	    default:
	       break;
	 }
      }
   }

   return TRUE;
}



/* helper for checking which drivers are valid */
void detect_sound()
{
   int i;

   for (i=0; digi_cards[i].id != DIGI_NONE; i++) {
      if (detect_digi_driver(digi_cards[i].id) == 0)
	 digi_cards[i].present = FALSE;
      else
	 digi_cards[i].present = TRUE;
   }
   digi_cards[i].present = TRUE;

   for (i=0; midi_cards[i].id != MIDI_NONE; i++) {
      if (detect_midi_driver(midi_cards[i].id) == 0)
	 midi_cards[i].present = FALSE;
      else
	 midi_cards[i].present = TRUE;
   }
   midi_cards[i].present = TRUE;
}



/* helper for initialising the sound code */
int init_sound(char *msg)
{
   char b1[80], b2[80];
   int i;

   detect_sound();

   if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) == 0)
      return 0;

   if (strlen(allegro_error) <= 32) {
      strcpy(b1, allegro_error);
      b2[0] = 0;
   }
   else {
      for (i=strlen(allegro_error)*9/16; i>10; i--)
	 if (allegro_error[i] == ' ')
	    break;

      strncpy(b1, allegro_error, i);
      b1[i] = 0;

      strcpy(b2, allegro_error+i+1);
   }

   alert(msg, b1, b2, "Ok", NULL, 0, 0);

   return -1;
}



BITMAP *popup_bitmap = NULL;
BITMAP *popup_bitmap2 = NULL;



/* helper for displaying a popup message */
void popup(char *s1, char *s2)
{
   int w, w2;

   if (!popup_bitmap) {
      for (w=512; w>=0; w-=2) {
	 line(screen, w, 16, 0, 16+w, 0);
	 if (!(w&15))
	    vsync();
      }
      for (w=0; w<512; w+=2) {
	 line(screen, w+1, 16, 0, 17+w, 0);
	 if (!(w&15))
	    vsync();
      }
      popup_bitmap = create_bitmap(SCREEN_W, SCREEN_H);
      popup_bitmap2 = create_bitmap(SCREEN_W, SCREEN_H);
      blit(screen, popup_bitmap, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
   }

   blit(popup_bitmap, popup_bitmap2, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

   if ((s1) || (s2)) {
      if (s1)
	 w = text_length(font, s1);
      else
	 w = 0;

      if (s2)
	 w2 = text_length(font, s2);
      else
	 w2 = 0;

      w = MAX(w, w2);

      rectfill(popup_bitmap2, (SCREEN_W-w)/2-15, SCREEN_H/2-31, (SCREEN_W+w)/2+15, SCREEN_H/2+31, 0);
      rect(popup_bitmap2, (SCREEN_W-w)/2-16, SCREEN_H/2-32, (SCREEN_W+w)/2+16, SCREEN_H/2+32, 255);

      text_mode(-1);

      if (s1)
	 textout_centre(popup_bitmap2, font, s1, SCREEN_W/2, SCREEN_H/2-20, -1);

      if (s2)
	 textout_centre(popup_bitmap2, font, s2, SCREEN_W/2, SCREEN_H/2+4, -1);
   }

   blit(popup_bitmap2, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}



/* ends the display of a popup message */
void end_popup()
{
   if (popup_bitmap) {
      blit(popup_bitmap, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
      destroy_bitmap(popup_bitmap);
      popup_bitmap = NULL;
      destroy_bitmap(popup_bitmap2);
      popup_bitmap2 = NULL;
   }
}



/* joystick test display */
void plot_joystick_state(BITMAP *bmp, int i)
{
   char buf[80];
   int j, x, y;
   int c = 0;

   text_mode(-1);

   if (joystick_driver) {
      if (num_joysticks > 1)
	 textprintf(bmp, font, 64, 40+c*20, -1, "%s (%d/%d)", joystick_driver->name, i+1, num_joysticks);
      else
	 textprintf(bmp, font, 64, 40+c*20, -1, joystick_driver->name);
      c++;
   }

   for (j=0; j<joy[i].num_sticks; j++) {
      if (joy[i].stick[j].num_axis == 2) {
	 if (joy[i].stick[j].flags & JOYFLAG_ANALOGUE) {
	    x = (joy[i].stick[j].axis[0].pos+128)*SCREEN_W/256;
	    y = (joy[i].stick[j].axis[1].pos+128)*SCREEN_H/256;
	    hline(bmp, x-12, y, x+12, 15);
	    vline(bmp, x, y-12, y+12, 15);
	    circle(bmp, x, y, 12, 1);
	    circlefill(bmp, x, y, 4, 31);
	 }
	 else {
	    sprintf(buf, "%s:", joy[i].stick[j].name);
	    if (joy[i].stick[j].axis[1].d1) 
	       strcat(buf, " up");
	    if (joy[i].stick[j].axis[1].d2) 
	       strcat(buf, " down");
	    if (joy[i].stick[j].axis[0].d1) 
	       strcat(buf, " left");
	    if (joy[i].stick[j].axis[0].d2) 
	       strcat(buf, " right");
	    textout(bmp, font, buf, 64, 40+c*20, -1);
	    c++;
	 }
      }
      else {
	 sprintf(buf, "%s: %s %4d %s", 
		joy[i].stick[j].name, 
		(joy[i].stick[j].axis[0].d1) ? "<-" : "  ", 
		joy[i].stick[j].axis[0].pos, 
		(joy[i].stick[j].axis[0].d2) ? "->" : "  ");

	 textout(bmp, font, buf, 64, 40+c*20, -1);
	 c++;
      }
   }

   for (j=0; j<joy[i].num_buttons; j++) {
      sprintf(buf, "%s: %s", joy[i].button[j].name, (joy[i].button[j].b) ? "*" : "");

      if (j&1) {
	 textout(bmp, font, buf, 192, 40+c*20, -1);
	 c++;
      }
      else
	 textout(bmp, font, buf, 64, 40+c*20, -1);
   }

   textout_centre(bmp, font, "- press a key to accept -", SCREEN_W/2, SCREEN_H-16, 255);
}



/* helper for calibrating the joystick */
void joystick_proc(int type)
{
   int i, c;

   scroller_proc(MSG_RADIO, NULL, 0);
   scare_mouse();

   remove_joystick();

   if (install_joystick(type) != 0) {
      alert("Error:", allegro_error, NULL, "Ok", NULL, 0, 0);
      unscare_mouse();
      return;
   }

   for (i=0; i<num_joysticks; i++) {
      while (joy[i].flags & JOYFLAG_CALIBRATE) {
	 popup(calibrate_joystick_name(i), "and press a button");

	 rest(10);
	 do {
	    poll_joystick();
	 } while ((!joy[i].button[0].b) && (!joy[i].button[1].b) && (!keypressed()));

	 if (calibrate_joystick(i) != 0) {
	    remove_joystick();
	    alert("Error calibrating joystick", NULL, NULL, "Ok", NULL, 0, 0);
	    end_popup();
	    unscare_mouse();
	    return;
	 }

	 rest(10);
	 do {
	    poll_joystick();
	 } while ((joy[i].button[0].b) || (joy[i].button[1].b));

	 clear_keybuf();
      }
   }

   if (!popup_bitmap)
      popup(NULL, NULL);

   do {
   } while (mouse_b);

   clear_keybuf();

   i = 0;
   c = 0;

   while ((!c) && (!mouse_b)) {
      poll_joystick();

      blit(popup_bitmap, popup_bitmap2, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
      plot_joystick_state(popup_bitmap2, i);
      blit(popup_bitmap2, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

      if (keypressed()) {
	 c = readkey();

	 switch (c>>8) {

	    case KEY_UP:
	    case KEY_RIGHT:
	       i++;
	       c = 0;
	       break;

	    case KEY_DOWN:
	    case KEY_LEFT:
	       i--;
	       c = 0;
	       break;

	    default:
	       c &= 0xFF;
	       break;
	 }

	 if (i >= num_joysticks)
	    i = num_joysticks-1;

	 if (i < 0)
	    i = 0;
      }
   }

   end_popup();

   do {
   } while (mouse_b);

   clear_keybuf();

   unscare_mouse();
}



/* dialog callback for retrieving the SB frequency list */
char *freq_getter(int index, int *list_size)
{
   static char *sb_freq[] =
   {
      "11906 hz",
      "16129 hz",
      "22727 hz",
      "45454 hz"
   };

   static char *ess_freq[] =
   {
      "11363 hz",
      "17046 hz",
      "22729 hz",
      "44194 hz"
   };

   static char *esc_freq[] =
   {
      "11025 hz",
      "16000 hz",
      "22050 hz",
      "48000 hz"
   };

   if (index < 0) {
      if (list_size) {
	 switch (soundcard->id) {

	    case DIGI_SB10:
	    case DIGI_SB15:
	       *list_size = 2;
	       break;

	    case DIGI_SBPRO:
	       *list_size = 3;
	       break;

	    case DIGI_SB:
	    case DIGI_SB20:
	    case DIGI_SB16:
	    case DIGI_AUDIODRIVE:
	    case DIGI_SOUNDSCAPE:
	    default:
	       *list_size = 4;
	       break;
	 }
      }
      return NULL;
   }

   if (soundcard->id == DIGI_AUDIODRIVE)
      return ess_freq[index];
   else if (soundcard->id == DIGI_SOUNDSCAPE)
      return esc_freq[index];
   else
      return sb_freq[index];
}



/* dialog callback for retrieving information about the soundcard list */
char *card_getter(int index, int *list_size)
{
   static char buf[80];
   int i;

   if (index < 0) {
      i = 0;
      while (soundcard[i].id)
	 i++;
      if (list_size)
	 *list_size = i+1;
      return NULL;
   }

   if (soundcard[index].present)
      sprintf(buf, "%c %s", 251, soundcard[index].name);
   else
      sprintf(buf, "  %s", soundcard[index].name);

   return buf;
}



char keyboard_type[256] = "";

int num_keyboard_layouts = 0;
char *keyboard_layouts[256];
char *keyboard_names[256];



char language_type[256] = "";

int num_language_layouts = 0;
char *language_layouts[256];
char *language_names[256];



/* helper for sorting the keyboard list */
void sort_keyboards()
{
   int done, i;

   do {
      done = TRUE;

      for (i=0; i<num_keyboard_layouts-1; i++) {
	 if (stricmp(keyboard_names[i], keyboard_names[i+1]) > 0) {
	    char *tl = keyboard_layouts[i];
	    char *tn = keyboard_names[i];

	    keyboard_layouts[i] = keyboard_layouts[i+1];
	    keyboard_names[i] = keyboard_names[i+1];

	    keyboard_layouts[i+1] = tl;
	    keyboard_names[i+1] = tn;

	    done = FALSE;
	 }
      }

   } while (!done);
}



/* helper for sorting the language list */
void sort_languages()
{
   int done, i;

   do {
      done = TRUE;

      for (i=0; i<num_language_layouts-1; i++) {
	 if (stricmp(language_names[i], language_names[i+1]) > 0) {
	    char *tl = language_layouts[i];
	    char *tn = language_names[i];

	    language_layouts[i] = language_layouts[i+1];
	    language_names[i] = language_names[i+1];

	    language_layouts[i+1] = tl;
	    language_names[i+1] = tn;

	    done = FALSE;
	 }
      }

   } while (!done);
}



/* dialog callback for retrieving information about the keyboard list */
char *keyboard_getter(int index, int *list_size)
{
   if (index < 0) {
      if (list_size)
	 *list_size = num_keyboard_layouts;
      return NULL;
   }

   return keyboard_names[index];
}



/* dialog callback for retrieving information about the language list */
char *language_getter(int index, int *list_size)
{
   if (index < 0) {
      if (list_size)
	 *list_size = num_language_layouts;
      return NULL;
   }

   return language_names[index];
}



/* dialog callback for retrieving information about the joystick list */
char *joystick_getter(int index, int *list_size)
{
   if (index < 0) {
      *list_size = sizeof(joystick_list)/sizeof(joystick_list[0]);
      return NULL;
   }

   return joystick_list[index].name;
}



/* dialog procedure for the soundcard selection listbox */
int card_proc(int msg, DIALOG *d, int c)
{
   int ret = d_list_proc(msg, d, c);
   d->dp3 = soundcard[d->d1].desc;
   return ret;
}



/* dialog procedure for the filename selection objects */
int filename_proc(int msg, DIALOG *d, int c)
{
   PARAMETER *p = d->dp2;
   char buf[256];
   char buf2[256];
   int ret;
   int i;

   if (msg == MSG_START) {
      if (!p->e2)
	 p->e2 = malloc(80);
      strcpy(p->e2, p->value);
   }
   else if (msg == MSG_END) {
      if (p->e2) {
	 free(p->e2);
	 p->e2 = NULL;
      }
   }

   ret = d_check_proc(msg, d, c);

   if (ret & D_CLOSE) {
      if (p->value[0]) {
	 strcpy(p->e2, p->value);
	 p->value[0] = 0;
      }
      else {
	 scroller_proc(MSG_RADIO, NULL, 0);

	 strcpy(buf2, p->desc);

	 for (i=1; buf2[i]; i++) {
	    if (buf2[i] == '(') {
	       buf2[i-1] = 0;
	       break;
	    }
	 }

	 strcpy(buf, p->e2);

	 if (file_select(buf2, buf, p->e1)) {
	    strcpy(p->value, buf);
	    strcpy(p->e2, buf);
	 }

	 scare_mouse();
	 blit(&background, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	 text_mode(0);
	 textout_centre(screen, font, TITLE, SCREEN_W/2, 0, -1);
	 unscare_mouse();
      }

      if (p->value[0])
	 d->flags |= D_SELECTED;
      else
	 d->flags &= ~D_SELECTED;

      ret &= ~D_CLOSE;
      ret |= D_REDRAW;
   }

   return ret;
}



/* wrapper for d_list_proc() to free up the dp2 parameter */
int d_xlist_proc(int msg, DIALOG *d, int c)
{
   void *old_dp2;
   int ret;

   old_dp2 = d->dp2;
   d->dp2 = NULL;

   ret = d_list_proc(msg, d, c);

   d->dp2 = old_dp2;

   return ret;
}



char backup_str[] =  "Go back to the previous menu";
char midi_desc[160];
char digi_desc[160];



DIALOG main_dlg[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)     (d1)           (d2)     (dp)                          (p)      (help message) */
   { d_button_proc,     30,   32,   124,  22,   -1,   16,   0,       D_EXIT,     0,             0,       "Autodetect",                 NULL,    "Attempt to autodetect your soundcard (ie. guess :-)" },
   { d_button_proc,     166,  32,   124,  22,   -1,   16,   0,       D_EXIT,     0,             0,       "Test",                       NULL,    "Test the current settings" },
   { d_button_proc,     30,   60,   124,  22,   -1,   16,   0,       D_EXIT,     0,             0,       "Digital Driver",             NULL,    "Manually select a driver for playing digital samples" },
   { d_button_proc,     166,  60,   124,  22,   -1,   16,   0,       D_EXIT,     0,             0,       "Midi Driver",                NULL,    "Manually select a driver for playing MIDI music" },
   { d_button_proc,     30,   88,   124,  22,   -1,   16,   0,       D_EXIT,     0,             0,       "Locale",                     NULL,    "Select a keyboard layout and system language" },
   { d_button_proc,     166,  88,   124,  22,   -1,   16,   0,       D_EXIT,     0,             0,       "Joystick",                   NULL,    "Calibrate your joystick" },
   { d_button_proc,     30,   116,  260,  22,   -1,   16,   0,       D_EXIT,     0,             0,       "Save changes and Exit",      NULL,    "Exit from the program, saving the current settings into the file '" CFG_FILE "'" },
   { d_button_proc,     30,   144,  260,  22,   -1,   16,   0,       D_EXIT,     0,             0,       "Quit (abandoning changes)",  NULL,    "Exit from the program, without saving the current settings" },
   { scroller_proc,     0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                         NULL,    NULL },
   { NULL,              0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                         NULL,    NULL }
};



DIALOG test_dlg[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)     (d1)           (d2)     (dp)                          (p)      (help message) */
   { d_button_proc,     100,  50,   120,  24,   -1,   16,   0,       D_EXIT,     0,             0,       "MIDI",                       NULL,    midi_desc },
   { d_button_proc,     30,   87,   80,   24,   -1,   16,   0,       D_EXIT,     0,             0,       "Left",                       NULL,    digi_desc },
   { d_button_proc,     120,  87,   80,   24,   -1,   16,   0,       D_EXIT,     0,             0,       "Centre",                     NULL,    digi_desc },
   { d_button_proc,     210,  87,   80,   24,   -1,   16,   0,       D_EXIT,     0,             0,       "Right",                      NULL,    digi_desc },
   { d_button_proc,     100,  124,  120,  24,   -1,   16,   0,       D_EXIT,     0,             0,       "Exit",                       NULL,    backup_str },
   { scroller_proc,     0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                         NULL,    NULL },
   { NULL,              0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                         NULL,    NULL }
};



DIALOG card_dlg[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)     (d1)           (d2)     (dp)                          (p)      (help message) */
   { d_button_proc,     30,   132,  124,  24,   -1,   16,   0,       D_EXIT,     0,             0,       "OK",                         NULL,    "Use this driver" },
   { d_button_proc,     166,  132,  124,  24,   -1,   16,   0,       D_EXIT,     0,             0,       "Cancel",                     NULL,    backup_str },
   { card_proc,         60,   36,   200,  83,   255,  16,   0,       D_EXIT,     0,             0,       card_getter,                  NULL,    NULL },
   { scroller_proc,     0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                         NULL,    NULL },
   { NULL,              0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                         NULL,    NULL }
};



DIALOG locale_dlg[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)     (d1)           (d2)     (dp)                          (p)      (help message) */
   { d_button_proc,     30,   132,  124,  24,   -1,   16,   0,       D_EXIT,     0,             0,       "OK",                         NULL,    "Use this keyboard layout and language" },
   { d_button_proc,     166,  132,  124,  24,   -1,   16,   0,       D_EXIT,     0,             0,       "Cancel",                     NULL,    backup_str },
   { d_list_proc,       8,    50,   146,  67,   255,  16,   0,       D_EXIT,     0,             0,       keyboard_getter,              NULL,    "Select a keyboard layout" },
   { d_list_proc,       166,  50,   146,  67,   255,  16,   0,       D_EXIT,     0,             0,       language_getter,              NULL,    "Select language for system messages" },
   { d_ctext_proc,      81,   30,   0,    0,    16,   -1,   0,       0,          0,             0,       "Keyboard",                   NULL,    NULL },
   { d_ctext_proc,      239,  30,   0,    0,    16,   -1,   0,       0,          0,             0,       "Language",                   NULL,    NULL },
   { scroller_proc,     0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                         NULL,    NULL },
   { NULL,              0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                         NULL,    NULL }
};



DIALOG joystick_dlg[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)     (d1)           (d2)     (dp)                          (p)      (help message) */
   { d_button_proc,     30,   132,  124,  24,   -1,   16,   0,       D_EXIT,     0,             0,       "OK",                         NULL,    "Use this joystick type" },
   { d_button_proc,     166,  132,  124,  24,   -1,   16,   0,       D_EXIT,     0,             0,       "Cancel",                     NULL,    backup_str },
   { d_list_proc,       60,   36,   200,  83,   255,  16,   0,       D_EXIT,     0,             0,       joystick_getter,              NULL,    "Select a type of joystick" },
   { scroller_proc,     0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                         NULL,    NULL },
   { NULL,              0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                         NULL,    NULL }
};



/* this one is generated depending on which parameters we need */
DIALOG param_dlg[32];

int param_ok;



/* handle input from the parameter dialog */
DIALOG_STATE param_handler(int c)
{
   PARAMETER *p;
   DIALOG *d = param_dlg;
   int i;

   if (c == param_ok) {
      /* save the changes */ 
      while (d->proc) {
	 p = d->dp2;

	 if (p) {
	    switch (p->type) {

	       case param_int:
		  if (p->value[0])
		     i = strtol(p->value, NULL, 0);
		  else
		     i = -1;
		  set_config_int("sound", p->name, i);
		  strcpy(p->value, get_config_string("sound", p->name, ""));
		  break;

	       case param_hex:
		  if (p->value[0])
		     i = strtol(p->value, NULL, 16);
		  else
		     i = -1;
		  set_config_hex("sound", p->name, i);
		  strcpy(p->value, get_config_string("sound", p->name, ""));
		  break;

	       case param_id:
		  if (p->value[0])
		     i = strtol(p->value, NULL, 0);
		  else
		     i = -1;
		  set_config_id("sound", p->name, i);
		  strcpy(p->value, get_config_string("sound", p->name, ""));
		  break;

	       case param_bool:
		  set_config_int("sound", p->name, (d->flags & D_SELECTED) ? 1 : 0);
		  strcpy(p->value, get_config_string("sound", p->name, ""));
		  break;

	       case param_file:
		  set_config_string("sound", p->name, p->value);
		  strcpy(p->value, get_config_string("sound", p->name, ""));
		  break;

	       case param_list:
		  i = strtol(freq_getter(d->d1, NULL), NULL, 0);
		  set_config_int("sound", p->name, i);
		  strcpy(p->value, get_config_string("sound", p->name, ""));
		  break;

	       default:
		  break;
	    }

	 }

	 d++;
      }
   }
   else {
      /* discard the changes */ 
      while (d->proc) {
	 p = d->dp2;

	 if (p)
	    strcpy(p->value, get_config_string("sound", p->name, ""));

	 d++;
      }
   }

   return state_exit;
}



/* sets up the soundcard parameter dialog box */
void setup_param_dialog()
{
   PARAMETER *p;
   DIALOG *d = param_dlg;
   char **c = soundcard->param;
   char *s;
   int pos = 0;
   int xo = 0;
   int yo = 0;
   int i, x, y, f, g;

   #define DLG(_p, _x, _y, _w, _h, _f, _b, _k, _l, _d1, _d2, _dp, _pa, _m) \
   {                                                                       \
      d->proc = _p;                                                        \
      d->x = _x;                                                           \
      d->y = _y;                                                           \
      d->w = _w;                                                           \
      d->h = _h;                                                           \
      d->fg = _f;                                                          \
      d->bg = _b;                                                          \
      d->key = _k;                                                         \
      d->flags = _l;                                                       \
      d->d1 = _d1;                                                         \
      d->d2 = _d2;                                                         \
      d->dp = _dp;                                                         \
      d->dp2 = _pa;                                                        \
      d->dp3 = _m;                                                         \
      d++;                                                                 \
   }

   while (*c) {
      if ((isdigit((*c)[0])) && (isdigit((*c)[1]))) {
	 xo = (*c)[0] - '0';
	 if (xo)
	    xo = 100 / xo;

	 yo = (*c)[1] - '0';
	 if (yo)
	    yo = 38 / yo;
      }
      else {
	 x = 16 + (pos%3) * 100 + xo;
	 y = 30 + (pos/3) * 38 + yo;
	 pos++;

	 p = NULL;

	 for (i=0; parameters[i].name; i++) {
	    if (stricmp(parameters[i].name, *c) == 0) {
	       p = &parameters[i];
	       break;
	    }
	 }

	 if (p) {
	    switch (p->type) {

	       case param_int:
		  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)           (p)         (help) */
		  DLG(d_box_proc,      x,    y,    88,   21,   255,  16,   0,    0,       0,    0,    NULL,          NULL,       NULL);
		  DLG(d_text_proc,     x+4,  y+3,  0,    0,    255,  16,   0,    0,       0,    0,    p->label,      NULL,       NULL);
		  DLG(d_edit_proc,     x+54, y+3,  32,   16,   255,  16,   0,    0,       3,    0,    p->value,      p,          p->desc);
		  break;

	       case param_hex:
		  if (stricmp(p->value, "FFFFFFFF") == 0)
		     strcpy(p->value, "-1");

		  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)           (p)         (help) */
		  DLG(d_box_proc,      x,    y,    88,   21,   255,  16,   0,    0,       0,    0,    NULL,          NULL,       NULL);
		  DLG(d_text_proc,     x+4,  y+3,  0,    0,    255,  16,   0,    0,       0,    0,    p->label,      NULL,       NULL);
		  DLG(d_edit_proc,     x+54, y+3,  32,   16,   255,  16,   0,    0,       3,    0,    p->value,      p,          p->desc);
		  break;

	       case param_bool:
		  if (strtol(p->value, NULL, 0) != 0)
		     f = D_SELECTED;
		  else
		     f = 0;

		  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)           (p)         (help) */
		  DLG(d_box_proc,      x,    y,    88,   21,   255,  16,   0,    0,       0,    0,    NULL,          NULL,       NULL);
		  DLG(d_text_proc,     x+4,  y+3,  0,    0,    255,  16,   0,    0,       0,    0,    p->label,      NULL,       NULL);
		  DLG(d_check_proc,    x+54, y+3,  31,   15,   255,  16,   0,    f,       0,    0,    " ",           p,          p->desc);
		  break;

	       case param_file:
		  if (p->value[0])
		     f = D_SELECTED | D_EXIT;
		  else
		     f = D_EXIT;

		  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)           (p)         (help) */
		  DLG(d_box_proc,      x,    y,    88,   21,   255,  16,   0,    0,       0,    0,    NULL,          NULL,       NULL);
		  DLG(d_text_proc,     x+4,  y+3,  0,    0,    255,  16,   0,    0,       0,    0,    p->label,      NULL,       NULL);
		  DLG(filename_proc,   x+62, y+3,  31,   15,   255,  16,   0,    f,       0,    0,    "",            p,          p->desc);
		  break;

	       case param_list:
		  i = strtol(p->value, NULL, 0);
		  freq_getter(-1, &f);
		  if (i > 0) {
		     for (g=0; g<f; g++) {
			s = freq_getter(g, NULL);
			if (i <= strtol(s, NULL, 0))
			   break;
		     }
		  }
		  else
		     g = 2;

		  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)           (p)         (help) */
		  DLG(d_xlist_proc,    x,    y-8,  88,   67,   255,  16,   0,    0,       g,    0,    freq_getter,   p,          p->desc);
		  break;

	       default:
		  break;
	    }
	 }
      }

      c++;
   }

   param_ok = ((int)d - (int)param_dlg) / sizeof(DIALOG);

   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)        (p)                        (help) */
   DLG(d_button_proc,   30,   142,  124,  24,   -1,   16,   13,   D_EXIT,  0,    0,    "OK",       NULL,                      "Use these parameters");
   DLG(d_button_proc,   166,  142,  124,  24,   -1,   16,   0,    D_EXIT,  0,    0,    "Cancel",   NULL,                      backup_str);
   DLG(scroller_proc,   0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,       NULL,                      NULL);
   DLG(NULL,            0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,       NULL,                      NULL);

   activate_dialog(param_dlg, param_handler, TRUE);
}



/* handle input from the test dialog */
DIALOG_STATE test_handler(int c)
{
   switch (c) {

      case 0:
	 /* MIDI test */
	 play_midi(&test_midi, FALSE);
	 return state_redraw;

      case 1:
	 /* left pan */
	 play_sample(&test_sample, 255, 0, 1000, FALSE);
	 return state_redraw;

      case 2:
	 /* centre pan */
	 play_sample(&test_sample, 255, 128, 1000, FALSE);
	 return state_redraw;

      case 3:
	 /* right pan */
	 play_sample(&test_sample, 255, 255, 1000, FALSE);
	 return state_redraw;

      default:
	 /* quit */
	 remove_sound();
	 return state_exit;
   }

   return state_active;
}



/* handle input from the card selection dialog */
DIALOG_STATE card_handler(int c)
{
   int i;

   switch (c) {

      case 0:
      case 2:
	 /* select driver */
	 i = (soundcard == digi_cards) ? 0 : 1;
	 soundcard += card_dlg[2].d1;
	 set_config_id("sound", parameters[i].name, soundcard->id);
	 strcpy(parameters[i].value, get_config_string("sound", parameters[i].name, ""));
	 if (soundcard->param)
	    setup_param_dialog();
	 else
	    return state_exit;
	 break;

      default:
	 /* quit */
	 return state_exit;
   }

   return state_active;
}



/* handle input from the locale selection dialog */
DIALOG_STATE locale_handler(int c)
{
   switch (c) {

      case 0:
      case 2:
      case 3:
	 /* select driver */
	 if (locale_dlg[2].d1 < num_keyboard_layouts)
	    strcpy(keyboard_type, keyboard_layouts[locale_dlg[2].d1]);

	 if (locale_dlg[3].d1 < num_language_layouts) {
	    char tmp[128];

	    strcpy(language_type, language_layouts[locale_dlg[3].d1]);

	    push_config_state();
	    sprintf(tmp, "language = %s\n", language_type);
	    set_config_data(tmp, strlen(tmp));
	    _load_config_text();
	    pop_config_state();
	 }

	 return state_exit;

      default:
	 /* quit */
	 return state_exit;
   }

   return state_active;
}



/* handle input from the joystick selection dialog */
DIALOG_STATE joystick_handler(int c)
{
   switch (c) {

      case 0:
      case 2:
	 /* select joystick */
	 joystick_proc(joystick_list[joystick_dlg[2].d1].id);
	 return state_exit;

      default:
	 /* quit */
	 return state_exit;
   }

   return state_active;
}



/* handle input from the main dialog */
DIALOG_STATE main_handler(int c)
{
   char b1[256], b2[80];
   int i;
   char *s, *s2;
   DATAFILE *data;

   switch (c) {

      case 0:
	 /* autodetect */
	 scroller_proc(MSG_RADIO, NULL, 0);

	 for (i=0; parameters[i].name; i++) {
	    set_config_string("sound", parameters[i].name, "");
	    parameters[i].value[0] = 0;
	 }

	 reserve_voices(0, 0);

	 if (cpu_family <= 4)
	    _sound_hq = 0;
	 else if (cpu_family <= 5)
	    _sound_hq = 1;
	 else
	    _sound_hq = 2;

	 if (init_sound("Unable to autodetect!") != 0)
	    return state_redraw;

	 sprintf(b1, "Digital: %s", digi_driver->name);
	 sprintf(b2, "MIDI: %s", midi_driver->name);
	 alert("- detected hardware -", b1, b2, "Ok", NULL, 0, 0);

	 for (i=0; parameters[i].name; i++) {
	    if (parameters[i].detect) {
	       switch (parameters[i].type) {

		  case param_int:
		  case param_bool:
		  case param_list:
		     set_config_int("sound", parameters[i].name, *parameters[i].detect);
		     break;

		  case param_id:
		     set_config_id("sound", parameters[i].name, *parameters[i].detect);
		     break;

		  case param_hex:
		     set_config_hex("sound", parameters[i].name, *parameters[i].detect);
		     break;

		  default:
		     break;
	       }
	    }
	    else
	       set_config_string("sound", parameters[i].name, parameters[i].def);

	    strcpy(parameters[i].value, get_config_string("sound", parameters[i].name, ""));
	 }

	 remove_sound();
	 return state_redraw;

      case 1:
	 /* test */
	 scroller_proc(MSG_RADIO, NULL, 0);
	 if (init_sound("Sound initialization failed!") != 0)
	    return state_redraw;
	 sprintf(midi_desc, "Driver: %s        Description: %s", midi_driver->name, midi_driver->desc);
	 sprintf(digi_desc, "Driver: %s        Description: %s", digi_driver->name, digi_driver->desc);
	 activate_dialog(test_dlg, test_handler, FALSE);
	 break;

      case 2:
	 /* choose digital driver */
	 soundcard = digi_cards;
	 for (i=0; soundcard[i].id; i++)
	    if (soundcard[i].id == get_config_id("sound", "digi_card", DIGI_AUTODETECT))
	       break;
	 card_dlg[2].d1 = i;
	 activate_dialog(card_dlg, card_handler, FALSE);
	 break;

      case 3:
	 /* choose MIDI driver */
	 soundcard = midi_cards;
	 for (i=0; soundcard[i].id; i++)
	    if (soundcard[i].id == get_config_id("sound", "midi_card", MIDI_AUTODETECT))
	       break;
	 card_dlg[2].d1 = i;
	 activate_dialog(card_dlg, card_handler, FALSE);
	 break;

      case 4:
	 /* read list of keyboard mappings */
	 if (num_keyboard_layouts <= 0) {
	    data = load_datafile(KEYBOARD_FILE);
	    if (!data) {
	       s = getenv("ALLEGRO");
	       if (s) {
		  append_filename(b1, s, KEYBOARD_FILE, sizeof(b1));
		  data = load_datafile(b1);
	       }
	    }
	    if (!data) {
	       scroller_proc(MSG_RADIO, NULL, 0);
	       alert("Error reading " KEYBOARD_FILE, NULL, NULL, "Ok", NULL, 0, 0);
	    }
	    else {
	       for (i=0; data[i].type != DAT_END; i++) {
		  s = get_datafile_property(data+i, DAT_ID('N','A','M','E'));

		  if (s) {
		     s2 = strstr(s, "_CFG");

		     if ((s2) && (s2[4]==0)) {
			s2 = keyboard_layouts[num_keyboard_layouts] = malloc(strlen(s)+1);
			strcpy(s2, s);
			s2[strlen(s2)-4] = 0;

			push_config_state();
			set_config_data(data[i].dat, data[i].size);
			s = get_config_string(NULL, "keyboard_name", s2);
			s2 = keyboard_names[num_keyboard_layouts] = malloc(strlen(s)+1);
			strcpy(s2, s);
			pop_config_state();

			num_keyboard_layouts++;
		     }
		  }
	       }
	       unload_datafile(data);
	    }
	 }

	 /* find the currently selected keyboard mapping */
	 if (num_keyboard_layouts > 0) {
	    sort_keyboards();

	    for (i=0; i<num_keyboard_layouts; i++)
	       if (stricmp(keyboard_type, keyboard_layouts[i]) == 0)
		  break;

	    if (i>=num_keyboard_layouts) {
	       sprintf(b1, "(%s)", keyboard_type);
	       scroller_proc(MSG_RADIO, NULL, 0);
	       alert("Warning: current keyboard", b1, "not found in " KEYBOARD_FILE, "Ok", NULL, 0, 0);
	       keyboard_layouts[num_keyboard_layouts] = malloc(strlen(keyboard_type)+1);
	       strcpy(keyboard_layouts[num_keyboard_layouts], keyboard_type);
	       keyboard_names[num_keyboard_layouts] = malloc(strlen(keyboard_type)+1);
	       strcpy(keyboard_names[num_keyboard_layouts], keyboard_type);
	       num_keyboard_layouts++;
	    }

	    locale_dlg[2].d1 = i;
	 }

	 /* read list of languages */
	 if (num_language_layouts <= 0) {
	    data = load_datafile(LANGUAGE_FILE);
	    if (!data) {
	       s = getenv("ALLEGRO");
	       if (s) {
		  append_filename(b1, s, LANGUAGE_FILE, sizeof(b1));
		  data = load_datafile(b1);
	       }
	    }
	    if (!data) {
	       scroller_proc(MSG_RADIO, NULL, 0);
	       alert("Error reading " LANGUAGE_FILE, NULL, NULL, "Ok", NULL, 0, 0);
	    }
	    else {
	       for (i=0; data[i].type != DAT_END; i++) {
		  s = get_datafile_property(data+i, DAT_ID('N','A','M','E'));

		  if (s) {
		     s2 = strstr(s, "TEXT_CFG");

		     if ((s2) && (s2[8]==0)) {
			s2 = language_layouts[num_language_layouts] = malloc(strlen(s)+1);
			strcpy(s2, s);
			s2[strlen(s2)-8] = 0;

			push_config_state();
			set_config_data(data[i].dat, data[i].size);
			s = get_config_string(NULL, "language_name", s2);
			s2 = language_names[num_language_layouts] = malloc(strlen(s)+1);
			strcpy(s2, s);
			pop_config_state();

			num_language_layouts++;
		     }
		  }
	       }
	       unload_datafile(data);
	    }
	 }

	 /* find the currently selected language mapping */
	 if (num_language_layouts > 0) {
	    sort_languages();

	    for (i=0; i<num_language_layouts; i++)
	       if (stricmp(language_type, language_layouts[i]) == 0)
		  break;

	    if (i>=num_language_layouts) {
	       sprintf(b1, "(%s)", language_type);
	       scroller_proc(MSG_RADIO, NULL, 0);
	       alert("Warning: current language", b1, "not found in " LANGUAGE_FILE, "Ok", NULL, 0, 0);
	       language_layouts[num_language_layouts] = malloc(strlen(language_type)+1);
	       strcpy(language_layouts[num_language_layouts], language_type);
	       language_names[num_language_layouts] = malloc(strlen(language_type)+1);
	       strcpy(language_names[num_language_layouts], language_type);
	       num_language_layouts++;
	    }

	    locale_dlg[3].d1 = i;
	 }

	 if ((num_keyboard_layouts > 0) || (num_language_layouts > 0))
	    activate_dialog(locale_dlg, locale_handler, FALSE);
	 break;

      case 5:
	 /* calibrate joystick */
	 joystick_dlg[2].d1 = 0;

	 for (i=0; i < (int)(sizeof(joystick_list)/sizeof(joystick_list[0])); i++) {
	    if (joystick_list[i].id == joy_type) {
	       joystick_dlg[2].d1 = i;
	       break;
	    }
	 }

	 activate_dialog(joystick_dlg, joystick_handler, FALSE);
	 break;

      case 6:
	 /* save settings and quit */
	 set_config_file(CFG_FILE);
	 for (i=0; parameters[i].name; i++) {
	    if (parameters[i].value[0])
	       set_config_string("sound", parameters[i].name, parameters[i].value);
	    else
	       set_config_string("sound", parameters[i].name, " ");
	 }
	 set_config_string(NULL, "keyboard", keyboard_type);
	 set_config_string(NULL, "language", language_type);
	 save_joystick_data(NULL);
	 return state_exit;

      default:
	 /* quit */
	 return state_exit;
   }

   return state_active;
}



int main()
{
   int i;

   allegro_init();
   install_mouse();
   install_keyboard();
   install_timer();
   check_cpu();

   fade_out(4);
   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);
   set_pallete(black_palette);

   set_mouse_range(0, 20, SCREEN_W-1, SCREEN_H-32);

   font = &setup_font;

   memset(scroller_msg, ' ', sizeof(scroller_msg));

   buffer = create_bitmap(SCREEN_W, SCREEN_H);

   LOCK_VARIABLE(scroller_time);
   LOCK_FUNCTION(inc_scroller_time);
   install_int_ex(inc_scroller_time, BPS_TO_TIMER(160));

   set_config_file(CFG_FILE);
   for (i=0; parameters[i].name; i++) {
      strcpy(parameters[i].value, get_config_string("sound", parameters[i].name, parameters[i].def));
      if (!parameters[i].value[0])
	 strcpy(parameters[i].value, parameters[i].def);
   }

   strcpy(keyboard_type, get_config_string(NULL, "keyboard", ""));
   for (i=0; keyboard_type[i]; i++)
      if (!isspace(keyboard_type[i]))
	 break;
   if (!keyboard_type[i])
      strcpy(keyboard_type, "us");

   strcpy(language_type, get_config_string(NULL, "language", ""));
   for (i=0; language_type[i]; i++)
      if (!isspace(language_type[i]))
	 break;
   if (!language_type[i])
      strcpy(language_type, "en");

   install_joystick(JOY_TYPE_AUTODETECT);

   set_config_data("", 0);
   for (i=0; parameters[i].name; i++)
      set_config_string("sound", parameters[i].name, parameters[i].value);

   detect_sound();

   activate_dialog(main_dlg, main_handler, FALSE);
   dialog_count++;

   do {
   } while (update());

   destroy_bitmap(buffer);

   for (i=0; i<num_keyboard_layouts; i++) {
      free(keyboard_layouts[i]);
      free(keyboard_names[i]);
   }

   for (i=0; i<num_language_layouts; i++) {
      free(language_layouts[i]);
      free(language_names[i]);
   }

   set_mouse_range(0, 0, SCREEN_W-1, SCREEN_H-1);

   return 0;
}


