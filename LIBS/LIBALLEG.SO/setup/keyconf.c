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
 *      The keyboard configuration utility program.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef DJGPP
#include <bios.h>
#include <dir.h>
#endif

#include "internal.h"



char *scancode_name[] = 
{
   "(zero)",
   "KEY_ESC",
   "KEY_1",
   "KEY_2",
   "KEY_3",
   "KEY_4",
   "KEY_5",
   "KEY_6",
   "KEY_7",
   "KEY_8",
   "KEY_9",
   "KEY_0",
   "KEY_MINUS",
   "KEY_EQUALS",
   "KEY_BACKSPACE",
   "KEY_TAB",
   "KEY_Q",
   "KEY_W",
   "KEY_E",
   "KEY_R",
   "KEY_T",
   "KEY_Y",
   "KEY_U",
   "KEY_I",
   "KEY_O",
   "KEY_P",
   "KEY_OPENBRACE",
   "KEY_CLOSEBRACE",
   "KEY_ENTER",
   "KEY_LCONTROL",
   "KEY_A",
   "KEY_S",
   "KEY_D",
   "KEY_F",
   "KEY_G",
   "KEY_H",
   "KEY_J",
   "KEY_K",
   "KEY_L",
   "KEY_COLON",
   "KEY_QUOTE",
   "KEY_TILDE",
   "KEY_LSHIFT",
   "KEY_BACKSLASH",
   "KEY_Z",
   "KEY_X",
   "KEY_C",
   "KEY_V",
   "KEY_B",
   "KEY_N",
   "KEY_M",
   "KEY_COMMA",
   "KEY_STOP",
   "KEY_SLASH",
   "KEY_RSHIFT",
   "KEY_ASTERISK",
   "KEY_ALT",
   "KEY_SPACE",
   "KEY_CAPSLOCK",
   "KEY_F1",
   "KEY_F2",
   "KEY_F3",
   "KEY_F4",
   "KEY_F5",
   "KEY_F6",
   "KEY_F7",
   "KEY_F8",
   "KEY_F9",
   "KEY_F10",
   "KEY_NUMLOCK",
   "KEY_SCRLOCK",
   "KEY_HOME",
   "KEY_UP",
   "KEY_PGUP",
   "KEY_MINUS_PAD",
   "KEY_LEFT",
   "KEY_5_PAD",
   "KEY_RIGHT",
   "KEY_PLUS_PAD",
   "KEY_END",
   "KEY_DOWN",
   "KEY_PGDN",
   "KEY_INSERT",
   "KEY_DEL",
   "KEY_PRTSCR",
   "(85)",
   "(86)",
   "KEY_F11",
   "KEY_F12",
   "(89)",
   "(90)",
   "KEY_LWIN",
   "KEY_RWIN",
   "KEY_MENU",
   "(94)",
   "(95)",
   "(96)",
   "(97)",
   "(98)",
   "(99)",
   "KEY_PAD",
   "(101)",
   "(102)",
   "(103)",
   "(104)",
   "(105)",
   "(106)",
   "(107)",
   "(108)",
   "(109)",
   "(110)",
   "(111)",
   "(112)",
   "(113)",
   "(114)",
   "(115)",
   "(116)",
   "(117)",
   "(118)",
   "(119)",
   "KEY_RCONTROL",
   "KEY_ALTGR",
   "KEY_SLASH2",
   "KEY_PAUSE",
   "(124)",
   "(125)",
   "(126)",
   "(127)"
};


unsigned char orig_key_ascii_table[128];
unsigned char orig_key_capslock_table[128];
unsigned char orig_key_shift_table[128];
unsigned char orig_key_control_table[128];
unsigned char orig_key_altgr_table[128];
unsigned char orig_key_accent1_lower_table[128];
unsigned char orig_key_accent1_upper_table[128];
unsigned char orig_key_accent1_shift_lower_table[128];
unsigned char orig_key_accent1_shift_upper_table[128];
unsigned char orig_key_accent2_lower_table[128];
unsigned char orig_key_accent2_upper_table[128];
unsigned char orig_key_accent2_shift_lower_table[128];
unsigned char orig_key_accent2_shift_upper_table[128];
unsigned char orig_key_numlock_table[128];
unsigned char orig_key_extended_table[128];
unsigned short orig_key_special_table[128];


char keyboard_name[64] = "";


char config_file[256] = "";


int loader();
int saver();
int quitter();
int editor();
int accenter1();
int accenter2();
int tester();


MENU file_menu[] =
{
   { "&Load\t(ctrl+L)",             loader,           NULL, 0, NULL },
   { "&Save\t(ctrl+S)",             saver,            NULL, 0, NULL },
   { "",                            NULL,             NULL, 0, NULL },
   { "&Quit\t(ctrl+Q)",             quitter,          NULL, 0, NULL },
   { NULL,                          NULL,             NULL, 0, NULL }
};


MENU edit_menu[] =
{
   { "Normal",                      editor,           NULL, 0, NULL },
   { "Capslock",                    editor,           NULL, 0, NULL },
   { "Shifted",                     editor,           NULL, 0, NULL },
   { "Control",                     editor,           NULL, 0, NULL },
   { "Alt-GR",                      editor,           NULL, 0, NULL },
   { "Accent1",                     editor,           NULL, 0, NULL },
   { "Accent1 (caps)",              editor,           NULL, 0, NULL },
   { "Accent1+shift",               editor,           NULL, 0, NULL },
   { "Accent1+shift (caps)",        editor,           NULL, 0, NULL },
   { "Accent2",                     editor,           NULL, 0, NULL },
   { "Accent2 (caps)",              editor,           NULL, 0, NULL },
   { "Accent2+shift",               editor,           NULL, 0, NULL },
   { "Accent2+shift (caps)",        editor,           NULL, 0, NULL },
   { NULL,                          NULL,             NULL, 0, NULL }
};


MENU misc_menu[] =
{
   { "Accent #&1",                  accenter1,        NULL, 0, NULL },
   { "Accent #&2",                  accenter2,        NULL, 0, NULL },
   { "",                            NULL,             NULL, 0, NULL },
   { "&Test\t(ctrl+T)",             tester,           NULL, 0, NULL },
   { NULL,                          NULL,             NULL, 0, NULL }
};


MENU menu[] = 
{ 
   { "&File",                       NULL,             file_menu, 0, NULL },
   { "&Edit",                       NULL,             edit_menu, 0, NULL },
   { "&Misc",                       NULL,             misc_menu, 0, NULL },
   { NULL,                          NULL,             NULL,      0, NULL }
};



#define C(x)      (x - 'a' + 1)


DIALOG main_dlg[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)     (d1)           (d2)     (dp)                                      (dp2) (dp3) */
   { d_clear_proc,      0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                                     NULL, NULL  },
   { d_menu_proc,       0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       menu,                                     NULL, NULL  },
   { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    C('l'),  0,          0,             0,       loader,                                   NULL, NULL  },
   { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    C('s'),  0,          0,             0,       saver,                                    NULL, NULL  },
   { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    C('t'),  0,          0,             0,       tester,                                   NULL, NULL  },
   { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    C('q'),  0,          0,             0,       quitter,                                  NULL, NULL  },
   { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    27,      0,          0,             0,       quitter,                                  NULL, NULL  },
   { d_ctext_proc,      160,  80,   0,    0,    255,  0,    0,       0,          0,             0,       "Allegro Keyboard Setup Utility",         NULL, NULL  },
   { d_ctext_proc,      160,  100,  0,    0,    255,  0,    0,       0,          0,             0,       "Version " ALLEGRO_VERSION_STR,           NULL, NULL  },
   { d_ctext_proc,      160,  132,  0,    0,    255,  0,    0,       0,          0,             0,       "By Shawn Hargreaves, " ALLEGRO_DATE_STR, NULL, NULL  },
   { d_text_proc,       80,   200,  0,    0,    255,  0,    0,       0,          0,             0,       "Name:",                                  NULL, NULL  },
   { d_edit_proc,       128,  200,  192,  8,    255,  0,    0,       0,          23,            0,       keyboard_name,                            NULL, NULL  },
   { NULL,              0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                                     NULL, NULL  }
};



/* reads a specific keymapping table from the config file */
void load_table(unsigned char *table, char *section)
{
   char name[80];
   int i;

   for (i=0; i<128; i++) {
      sprintf(name, "key%d", i);
      table[i] = get_config_int(section, name, table[i]);
   }
}



/* handle the load command */
int loader()
{
   char buf[256];
   int c;

   strcpy(buf, config_file);

   if (file_select("Load Keyboard Config", buf, "CFG")) {
      strlwr(buf);
      strcpy(config_file, buf);

      memcpy(key_ascii_table, orig_key_ascii_table, sizeof(orig_key_ascii_table));
      memcpy(key_capslock_table, orig_key_capslock_table, sizeof(orig_key_capslock_table));
      memcpy(key_shift_table, orig_key_shift_table, sizeof(orig_key_shift_table));
      memcpy(key_control_table, orig_key_control_table, sizeof(orig_key_control_table));
      memcpy(key_altgr_table, orig_key_altgr_table, sizeof(orig_key_altgr_table));
      memcpy(key_accent1_lower_table, orig_key_accent1_lower_table, sizeof(orig_key_accent1_lower_table));
      memcpy(key_accent1_upper_table, orig_key_accent1_upper_table, sizeof(orig_key_accent1_upper_table));
      memcpy(key_accent1_shift_lower_table, orig_key_accent1_shift_lower_table, sizeof(orig_key_accent1_shift_lower_table));
      memcpy(key_accent1_shift_upper_table, orig_key_accent1_shift_upper_table, sizeof(orig_key_accent1_shift_upper_table));
      memcpy(key_accent2_lower_table, orig_key_accent2_lower_table, sizeof(orig_key_accent2_lower_table));
      memcpy(key_accent2_upper_table, orig_key_accent2_upper_table, sizeof(orig_key_accent2_upper_table));
      memcpy(key_accent2_shift_lower_table, orig_key_accent2_shift_lower_table, sizeof(orig_key_accent2_shift_lower_table));
      memcpy(key_accent2_shift_upper_table, orig_key_accent2_shift_upper_table, sizeof(orig_key_accent2_shift_upper_table));
      memcpy(key_numlock_table, orig_key_numlock_table, sizeof(orig_key_numlock_table));
      memcpy(key_extended_table, orig_key_extended_table, sizeof(orig_key_extended_table));
      memcpy(key_special_table, orig_key_special_table, sizeof(orig_key_special_table));

      push_config_state();
      set_config_file(buf);

      load_table(key_ascii_table,               "key_ascii");
      load_table(key_capslock_table,            "key_capslock");
      load_table(key_shift_table,               "key_shift");
      load_table(key_control_table,             "key_control");
      load_table(key_altgr_table,               "key_altgr");
      load_table(key_accent1_lower_table,       "key_accent1_lower");
      load_table(key_accent1_upper_table,       "key_accent1_upper");
      load_table(key_accent1_shift_lower_table, "key_accent1_shift_lower");
      load_table(key_accent1_shift_upper_table, "key_accent1_shift_upper");
      load_table(key_accent2_lower_table,       "key_accent2_lower");
      load_table(key_accent2_upper_table,       "key_accent2_upper");
      load_table(key_accent2_shift_lower_table, "key_accent2_shift_lower");
      load_table(key_accent2_shift_upper_table, "key_accent2_shift_upper");

      c = get_config_int("key_escape", "accent1", 0);
      if (c)
	 key_special_table[c&127] = KB_ACCENT1_FLAG;

      c = get_config_int("key_escape", "accent2", 0);
      if (c)
	 key_special_table[c&127] = KB_ACCENT2_FLAG;

      strcpy(keyboard_name, get_config_string(NULL, "keyboard_name", ""));
      main_dlg[11].d2 = strlen(keyboard_name);

      pop_config_state();
   }

   return D_REDRAW;
}



/* writes a specific keymapping table to the config file */
void save_table(unsigned char *table, unsigned char *origtable, char *section)
{
   char name[80];
   int i;

   for (i=0; i<128; i++) {
      sprintf(name, "key%d", i);
      if (table[i] != origtable[i])
	 set_config_int(section, name, table[i]);
      else
	 set_config_string(section, name, NULL);
   }
}



/* handle the save command */
int saver()
{
   char buf[256];
   int i;

   strcpy(buf, config_file);

   if (file_select("Save Keyboard Config", buf, "CFG")) {
      if ((stricmp(config_file, buf) != 0) && (exists(buf))) {
	 if (alert("Overwrite existing file?", NULL, NULL, "Yes", "Cancel", 'y', 27) != 1)
	    return D_REDRAW;
      }

      strlwr(buf);
      strcpy(config_file, buf);

      push_config_state();
      set_config_file(buf);

      set_config_string(NULL, "keyboard_name", keyboard_name);

      for (i=0; i<128; i++)
	 if (key_special_table[i] == KB_ACCENT1_FLAG)
	    break;

      set_config_int("key_escape", "accent1", (i<128 ? i : 0));

      for (i=0; i<128; i++)
	 if (key_special_table[i] == KB_ACCENT2_FLAG)
	    break;

      set_config_int("key_escape", "accent2", (i<128 ? i : 0));

      save_table(key_ascii_table,               orig_key_ascii_table,               "key_ascii");
      save_table(key_capslock_table,            orig_key_capslock_table,            "key_capslock");
      save_table(key_shift_table,               orig_key_shift_table,               "key_shift");
      save_table(key_control_table,             orig_key_control_table,             "key_control");
      save_table(key_altgr_table,               orig_key_altgr_table,               "key_altgr");
      save_table(key_accent1_lower_table,       orig_key_accent1_lower_table,       "key_accent1_lower");
      save_table(key_accent1_upper_table,       orig_key_accent1_upper_table,       "key_accent1_upper");
      save_table(key_accent1_shift_lower_table, orig_key_accent1_shift_lower_table, "key_accent1_shift_lower");
      save_table(key_accent1_shift_upper_table, orig_key_accent1_shift_upper_table, "key_accent1_shift_upper");
      save_table(key_accent2_lower_table,       orig_key_accent2_lower_table,       "key_accent2_lower");
      save_table(key_accent2_upper_table,       orig_key_accent2_upper_table,       "key_accent2_upper");
      save_table(key_accent2_shift_lower_table, orig_key_accent2_shift_lower_table, "key_accent2_shift_lower");
      save_table(key_accent2_shift_upper_table, orig_key_accent2_shift_upper_table, "key_accent2_shift_upper");

      pop_config_state();
   }

   return D_REDRAW;
}



/* handle the quit command */
int quitter()
{
   if (alert("Really want to quit?", NULL, NULL, "Yes", "Cancel", 'y', 27) == 1)
      return D_CLOSE;
   else
      return D_O_K;
}



/* dialog procedure for ASCII character objects */
int ascii_proc(int msg, DIALOG *d, int c)
{
   int fg, bg;

   if (msg == MSG_DRAW) {
      if (d->flags & D_SELECTED) {
	 fg = d->bg;
	 bg = d->fg;
      }
      else if (d->flags & D_GOTFOCUS) {
	 fg = d->fg;
	 bg = gui_mg_color;
      }
      else {
	 fg = d->fg;
	 bg = d->bg;
      }

      rectfill(screen, d->x+1, d->y+1, d->x+d->w-1, d->y+d->h-1, bg);
      rect(screen, d->x, d->y, d->x+d->w, d->y+d->h, fg);
      text_mode(-1);

      if (d->dp)
	 textout_centre(screen, font, d->dp, d->x+10, d->y+4, fg);
      else
	 textprintf(screen, font, d->x+6, d->y+4, fg, "%c", d->d1);

      return D_O_K;
   }

   return d_button_proc(msg, d, c);
}



#define ASCII_CHAR(n)            \
   { ascii_proc, (n&15)*20, (n/16)*15, 20, 15, 255, 0, 0, D_EXIT, n, 0, NULL, NULL, NULL }


#define ASCII_CHAR_EX(n, msg)    \
   { ascii_proc, (n&15)*20, (n/16)*15, 20, 15, 255, 0, 0, D_EXIT, n, 0, msg, NULL, NULL }


DIALOG ascii_dlg[] =
{
   ASCII_CHAR_EX(0x00, "\\0"), 
   ASCII_CHAR_EX(0x01, "^a"), 
   ASCII_CHAR_EX(0x02, "^b"), 
   ASCII_CHAR_EX(0x03, "^c"), 
   ASCII_CHAR_EX(0x04, "^d"), 
   ASCII_CHAR_EX(0x05, "^e"), 
   ASCII_CHAR_EX(0x06, "^f"), 
   ASCII_CHAR_EX(0x07, "^g"), 
   ASCII_CHAR_EX(0x08, "^h"), 
   ASCII_CHAR_EX(0x09, "^i"), 
   ASCII_CHAR_EX(0x0A, "^j"), 
   ASCII_CHAR_EX(0x0B, "^k"), 
   ASCII_CHAR_EX(0x0C, "^l"), 
   ASCII_CHAR_EX(0x0D, "^m"), 
   ASCII_CHAR_EX(0x0E, "^n"), 
   ASCII_CHAR_EX(0x0F, "^o"), 
   ASCII_CHAR_EX(0x10, "^p"), 
   ASCII_CHAR_EX(0x11, "^q"), 
   ASCII_CHAR_EX(0x12, "^r"), 
   ASCII_CHAR_EX(0x13, "^s"), 
   ASCII_CHAR_EX(0x14, "^t"), 
   ASCII_CHAR_EX(0x15, "^u"), 
   ASCII_CHAR_EX(0x16, "^v"), 
   ASCII_CHAR_EX(0x17, "^w"), 
   ASCII_CHAR_EX(0x18, "^x"), 
   ASCII_CHAR_EX(0x19, "^y"), 
   ASCII_CHAR_EX(0x1A, "^z"), 
   ASCII_CHAR_EX(0x1B, "27"), 
   ASCII_CHAR_EX(0x1C, "28"), 
   ASCII_CHAR_EX(0x1D, "29"), 
   ASCII_CHAR_EX(0x1E, "30"), 
   ASCII_CHAR_EX(0x1F, "31"), 
   ASCII_CHAR(0x20), ASCII_CHAR(0x21), ASCII_CHAR(0x22), ASCII_CHAR(0x23),
   ASCII_CHAR(0x24), ASCII_CHAR(0x25), ASCII_CHAR(0x26), ASCII_CHAR(0x27),
   ASCII_CHAR(0x28), ASCII_CHAR(0x29), ASCII_CHAR(0x2A), ASCII_CHAR(0x2B),
   ASCII_CHAR(0x2C), ASCII_CHAR(0x2D), ASCII_CHAR(0x2E), ASCII_CHAR(0x2F),
   ASCII_CHAR(0x30), ASCII_CHAR(0x31), ASCII_CHAR(0x32), ASCII_CHAR(0x33),
   ASCII_CHAR(0x34), ASCII_CHAR(0x35), ASCII_CHAR(0x36), ASCII_CHAR(0x37),
   ASCII_CHAR(0x38), ASCII_CHAR(0x39), ASCII_CHAR(0x3A), ASCII_CHAR(0x3B),
   ASCII_CHAR(0x3C), ASCII_CHAR(0x3D), ASCII_CHAR(0x3E), ASCII_CHAR(0x3F),
   ASCII_CHAR(0x40), ASCII_CHAR(0x41), ASCII_CHAR(0x42), ASCII_CHAR(0x43),
   ASCII_CHAR(0x44), ASCII_CHAR(0x45), ASCII_CHAR(0x46), ASCII_CHAR(0x47),
   ASCII_CHAR(0x48), ASCII_CHAR(0x49), ASCII_CHAR(0x4A), ASCII_CHAR(0x4B),
   ASCII_CHAR(0x4C), ASCII_CHAR(0x4D), ASCII_CHAR(0x4E), ASCII_CHAR(0x4F),
   ASCII_CHAR(0x50), ASCII_CHAR(0x51), ASCII_CHAR(0x52), ASCII_CHAR(0x53),
   ASCII_CHAR(0x54), ASCII_CHAR(0x55), ASCII_CHAR(0x56), ASCII_CHAR(0x57),
   ASCII_CHAR(0x58), ASCII_CHAR(0x59), ASCII_CHAR(0x5A), ASCII_CHAR(0x5B),
   ASCII_CHAR(0x5C), ASCII_CHAR(0x5D), ASCII_CHAR(0x5E), ASCII_CHAR(0x5F),
   ASCII_CHAR(0x60), ASCII_CHAR(0x61), ASCII_CHAR(0x62), ASCII_CHAR(0x63),
   ASCII_CHAR(0x64), ASCII_CHAR(0x65), ASCII_CHAR(0x66), ASCII_CHAR(0x67),
   ASCII_CHAR(0x68), ASCII_CHAR(0x69), ASCII_CHAR(0x6A), ASCII_CHAR(0x6B),
   ASCII_CHAR(0x6C), ASCII_CHAR(0x6D), ASCII_CHAR(0x6E), ASCII_CHAR(0x6F),
   ASCII_CHAR(0x70), ASCII_CHAR(0x71), ASCII_CHAR(0x72), ASCII_CHAR(0x73),
   ASCII_CHAR(0x74), ASCII_CHAR(0x75), ASCII_CHAR(0x76), ASCII_CHAR(0x77),
   ASCII_CHAR(0x78), ASCII_CHAR(0x79), ASCII_CHAR(0x7A), ASCII_CHAR(0x7B),
   ASCII_CHAR(0x7C), ASCII_CHAR(0x7D), ASCII_CHAR(0x7E), ASCII_CHAR(0x7F),
   ASCII_CHAR(0x80), ASCII_CHAR(0x81), ASCII_CHAR(0x82), ASCII_CHAR(0x83),
   ASCII_CHAR(0x84), ASCII_CHAR(0x85), ASCII_CHAR(0x86), ASCII_CHAR(0x87),
   ASCII_CHAR(0x88), ASCII_CHAR(0x89), ASCII_CHAR(0x8A), ASCII_CHAR(0x8B),
   ASCII_CHAR(0x8C), ASCII_CHAR(0x8D), ASCII_CHAR(0x8E), ASCII_CHAR(0x8F),
   ASCII_CHAR(0x90), ASCII_CHAR(0x91), ASCII_CHAR(0x92), ASCII_CHAR(0x93),
   ASCII_CHAR(0x94), ASCII_CHAR(0x95), ASCII_CHAR(0x96), ASCII_CHAR(0x97),
   ASCII_CHAR(0x98), ASCII_CHAR(0x99), ASCII_CHAR(0x9A), ASCII_CHAR(0x9B),
   ASCII_CHAR(0x9C), ASCII_CHAR(0x9D), ASCII_CHAR(0x9E), ASCII_CHAR(0x9F),
   ASCII_CHAR(0xA0), ASCII_CHAR(0xA1), ASCII_CHAR(0xA2), ASCII_CHAR(0xA3),
   ASCII_CHAR(0xA4), ASCII_CHAR(0xA5), ASCII_CHAR(0xA6), ASCII_CHAR(0xA7),
   ASCII_CHAR(0xA8), ASCII_CHAR(0xA9), ASCII_CHAR(0xAA), ASCII_CHAR(0xAB),
   ASCII_CHAR(0xAC), ASCII_CHAR(0xAD), ASCII_CHAR(0xAE), ASCII_CHAR(0xAF),
   ASCII_CHAR(0xB0), ASCII_CHAR(0xB1), ASCII_CHAR(0xB2), ASCII_CHAR(0xB3),
   ASCII_CHAR(0xB4), ASCII_CHAR(0xB5), ASCII_CHAR(0xB6), ASCII_CHAR(0xB7),
   ASCII_CHAR(0xB8), ASCII_CHAR(0xB9), ASCII_CHAR(0xBA), ASCII_CHAR(0xBB),
   ASCII_CHAR(0xBC), ASCII_CHAR(0xBD), ASCII_CHAR(0xBE), ASCII_CHAR(0xBF),
   ASCII_CHAR(0xC0), ASCII_CHAR(0xC1), ASCII_CHAR(0xC2), ASCII_CHAR(0xC3),
   ASCII_CHAR(0xC4), ASCII_CHAR(0xC5), ASCII_CHAR(0xC6), ASCII_CHAR(0xC7),
   ASCII_CHAR(0xC8), ASCII_CHAR(0xC9), ASCII_CHAR(0xCA), ASCII_CHAR(0xCB),
   ASCII_CHAR(0xCC), ASCII_CHAR(0xCD), ASCII_CHAR(0xCE), ASCII_CHAR(0xCF),
   ASCII_CHAR(0xD0), ASCII_CHAR(0xD1), ASCII_CHAR(0xD2), ASCII_CHAR(0xD3),
   ASCII_CHAR(0xD4), ASCII_CHAR(0xD5), ASCII_CHAR(0xD6), ASCII_CHAR(0xD7),
   ASCII_CHAR(0xD8), ASCII_CHAR(0xD9), ASCII_CHAR(0xDA), ASCII_CHAR(0xDB),
   ASCII_CHAR(0xDC), ASCII_CHAR(0xDD), ASCII_CHAR(0xDE), ASCII_CHAR(0xDF),
   ASCII_CHAR(0xE0), ASCII_CHAR(0xE1), ASCII_CHAR(0xE2), ASCII_CHAR(0xE3),
   ASCII_CHAR(0xE4), ASCII_CHAR(0xE5), ASCII_CHAR(0xE6), ASCII_CHAR(0xE7),
   ASCII_CHAR(0xE8), ASCII_CHAR(0xE9), ASCII_CHAR(0xEA), ASCII_CHAR(0xEB),
   ASCII_CHAR(0xEC), ASCII_CHAR(0xED), ASCII_CHAR(0xEE), ASCII_CHAR(0xEF),
   ASCII_CHAR(0xF0), ASCII_CHAR(0xF1), ASCII_CHAR(0xF2), ASCII_CHAR(0xF3),
   ASCII_CHAR(0xF4), ASCII_CHAR(0xF5), ASCII_CHAR(0xF6), ASCII_CHAR(0xF7),
   ASCII_CHAR(0xF8), ASCII_CHAR(0xF9), ASCII_CHAR(0xFA), ASCII_CHAR(0xFB),
   ASCII_CHAR(0xFC), ASCII_CHAR(0xFD), ASCII_CHAR(0xFE), ASCII_CHAR(0xFF),
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};



/* inputs an ASCII code */
int get_ascii_code(int *val)
{
   int ret;

   position_mouse(ascii_dlg[*val].x+ascii_dlg[*val].w/2, ascii_dlg[*val].y+ascii_dlg[*val].h/2);

   ret = do_dialog(ascii_dlg, *val);

   if ((ret >= 0) && (ret < 256)) {
      *val = ret;
      return TRUE;
   }

   return FALSE;
}



unsigned char *editor_table;



/* dialog callback for retrieving information about the keymap list */
char *keymap_list_getter(int index, int *list_size)
{
   static char *ascii_name[32] = 
   {
      "", "^a", "^b", "^c", "^d", "^e", "^f", "^g",
      "^h", "^i", "^j", "^k", "^l", "^m", "^n", "^o",
      "^p", "^q", "^r", "^s", "^t", "^u", "^v", "^w",
      "^x", "^y", "^z", "27", "28", "29", "30", "31"
   };

   static char buf[256];
   int val;

   if (index < 0) {
      if (list_size)
	 *list_size = 128;
      return NULL;
   }

   val = editor_table[index];

   if (val >= 32)
      sprintf(buf, "%02X: %-16s 0x%02X - '%c'", index, scancode_name[index], val, val);
   else
      sprintf(buf, "%02X: %-16s 0x%02X - %s", index, scancode_name[index], val, ascii_name[val]);

   return buf;
}



/* dialog procedure for the keymap listbox */
int keymap_list_proc(int msg, DIALOG *d, int c)
{
   int ret;

   if (msg == MSG_LOSTFOCUS)
      return D_WANTFOCUS;

   ret = d_list_proc(msg, d, c);

   if (ret & D_CLOSE) {
      int val = editor_table[d->d1];

      if (get_ascii_code(&val))
	 editor_table[d->d1] = val;

      return D_REDRAW;
   }

   return ret;
}



DIALOG editor_dlg[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)     (d1)           (d2)     (dp)                 (dp2) (dp3) */
   { d_clear_proc,      0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                NULL, NULL  },
   { d_ctext_proc,      160,  0,    0,    0,    255,  0,    0,       0,          0,             0,       NULL,                NULL, NULL  },
   { keymap_list_proc,  16,   24,   288,  203,  255,  0,    0,       D_EXIT,     0,             0,       keymap_list_getter,  NULL, NULL  },
   { NULL,              0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                NULL, NULL  }
};



/* core keymap editing function */
int editor()
{
   editor_table = active_menu->dp;
   editor_dlg[1].dp = active_menu->text;

   do_dialog(editor_dlg, -1);

   return D_REDRAW;
}



/* dialog callback for retrieving information about the accent key */
char *accent_list_getter(int index, int *list_size)
{
   static char buf[256];

   if (index < 0) {
      if (list_size)
	 *list_size = 128;
      return NULL;
   }

   sprintf(buf, "0x%02X - %s", index, scancode_name[index]);

   return buf;
}



DIALOG accent_dlg[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)     (d1)           (d2)     (dp)                 (dp2) (dp3) */
   { d_clear_proc,      0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                NULL, NULL  },
   { d_ctext_proc,      160,  0,    0,    0,    255,  0,    0,       0,          0,             0,       NULL,                NULL, NULL  },
   { d_list_proc,       16,   24,   288,  203,  255,  0,    0,       D_EXIT,     0,             0,       accent_list_getter,  NULL, NULL  },
   { NULL,              0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,                NULL, NULL  }
};



/* accent setting function */
int accenter(char *title, int flag)
{
   int ret;
   int i;

   accent_dlg[1].dp = title;

   for (i=0; i<128; i++)
      if (key_special_table[i] == flag)
	 break;

   accent_dlg[2].d1 = (i<128) ? i : 0;
   accent_dlg[2].d2 = 0;

   ret = do_dialog(accent_dlg, -1);

   if (ret == 2) {
      if ((key_special_table[accent_dlg[2].d1] == 0) ||
	  (key_special_table[accent_dlg[2].d1] == KB_ACCENT1_FLAG) ||
	  (key_special_table[accent_dlg[2].d1] == KB_ACCENT2_FLAG)) {
	 key_special_table[i] = 0;
	 key_special_table[accent_dlg[2].d1] = flag;
      }
      else {
	 alert("That key is not valid",  "for use as an accent!", NULL, "Sorry", NULL, 13, 0);
      }
   }

   return D_REDRAW;
}



int accenter1()
{
   return accenter("Set Accent Key #1", KB_ACCENT1_FLAG);
}



int accenter2()
{
   return accenter("Set Accent Key #2", KB_ACCENT2_FLAG);
}



/* handle the test command */
int tester()
{
   int a, i, x, y;
   char buf[256];

   show_mouse(NULL);
   clear(screen);

   text_mode(0);

   for (x=0; x<=16; x++) {
      if (x<16)
	 textprintf(screen, font, x*16+32, 88, 255, "%X", x);
      vline(screen, x*16+28, 90, 228, 255);
   }

   for (y=0; y<=8; y++) {
      if (y<8)
	 textprintf(screen, font, 16, y*16+104, 255, "%X", y);
      hline(screen, 18, y*16+100, 284, 255);
   }

   do {
   } while ((key[KEY_ESC]) || (mouse_b));

   do {
      for (i=0; i<128; i++)
	 textout(screen, font, key[i] ? "*" : " ", (i&15)*16+32, (i/16)*16+104, 255);

      buf[0] = 0;

      if (key_shifts & KB_SHIFT_FLAG)
	 strcat(buf, "shift ");

      if (key_shifts & KB_CTRL_FLAG)
	 strcat(buf, "ctrl ");

      if (key_shifts & KB_ALT_FLAG)
	 strcat(buf, "alt ");

      if (key_shifts & KB_LWIN_FLAG)
	 strcat(buf, "lwin ");

      if (key_shifts & KB_RWIN_FLAG)
	 strcat(buf, "rwin ");

      if (key_shifts & KB_MENU_FLAG)
	 strcat(buf, "menu ");

      if (key_shifts & KB_SCROLOCK_FLAG)
	 strcat(buf, "scrolock ");

      if (key_shifts & KB_NUMLOCK_FLAG)
	 strcat(buf, "numlock ");

      if (key_shifts & KB_CAPSLOCK_FLAG)
	 strcat(buf, "capslock ");

      if (key_shifts & KB_INALTSEQ_FLAG)
	 strcat(buf, "inaltseq ");

      if (key_shifts & KB_ACCENT1_FLAG)
	 strcat(buf, "accent1 ");

      if (key_shifts & KB_ACCENT1_S_FLAG)
	 strcat(buf, "accent1+shift ");

      if (key_shifts & KB_ACCENT2_FLAG)
	 strcat(buf, "accent2 ");

      if (key_shifts & KB_ACCENT2_S_FLAG)
	 strcat(buf, "accent2+shift ");

      while (strlen(buf) < 128)
	 strcat(buf, " ");

      textout(screen, font, buf, 0, 0, 255);

      if (keypressed()) {
	 i = readkey();
	 a = i&0xFF;
	 if (!a)
	    a = ' ';
	 textprintf(screen, font, 8, 32, 255, "readkey() returns 0x%04X - ASCII '%c'", i, a);
      }

   } while ((!key[KEY_ESC]) && (!mouse_b));

   do {
   } while ((key[KEY_ESC]) || (mouse_b));

   clear_keybuf();

   show_mouse(screen);
   return D_REDRAW;
}



int main()
{
   allegro_init();
   install_mouse();
   install_timer();
   install_keyboard();

   switch_standard_kb_key = 0;
   switch_custom_kb_key = 0;

   set_custom_keyboard();

   memcpy(orig_key_ascii_table, key_ascii_table, sizeof(orig_key_ascii_table));
   memcpy(orig_key_capslock_table, key_capslock_table, sizeof(orig_key_capslock_table));
   memcpy(orig_key_shift_table, key_shift_table, sizeof(orig_key_shift_table));
   memcpy(orig_key_control_table, key_control_table, sizeof(orig_key_control_table));
   memcpy(orig_key_altgr_table, key_altgr_table, sizeof(orig_key_altgr_table));
   memcpy(orig_key_accent1_lower_table, key_accent1_lower_table, sizeof(orig_key_accent1_lower_table));
   memcpy(orig_key_accent1_upper_table, key_accent1_upper_table, sizeof(orig_key_accent1_upper_table));
   memcpy(orig_key_accent1_shift_lower_table, key_accent1_shift_lower_table, sizeof(orig_key_accent1_shift_lower_table));
   memcpy(orig_key_accent1_shift_upper_table, key_accent1_shift_upper_table, sizeof(orig_key_accent1_shift_upper_table));
   memcpy(orig_key_accent2_lower_table, key_accent2_lower_table, sizeof(orig_key_accent2_lower_table));
   memcpy(orig_key_accent2_upper_table, key_accent2_upper_table, sizeof(orig_key_accent2_upper_table));
   memcpy(orig_key_accent2_shift_lower_table, key_accent2_shift_lower_table, sizeof(orig_key_accent2_shift_lower_table));
   memcpy(orig_key_accent2_shift_upper_table, key_accent2_shift_upper_table, sizeof(orig_key_accent2_shift_upper_table));
   memcpy(orig_key_numlock_table, key_numlock_table, sizeof(orig_key_numlock_table));
   memcpy(orig_key_extended_table, key_extended_table, sizeof(orig_key_extended_table));
   memcpy(orig_key_special_table, key_special_table, sizeof(orig_key_special_table));

   edit_menu[0].dp = key_ascii_table;
   edit_menu[1].dp = key_capslock_table;
   edit_menu[2].dp = key_shift_table;
   edit_menu[3].dp = key_control_table;
   edit_menu[4].dp = key_altgr_table;
   edit_menu[5].dp = key_accent1_lower_table;
   edit_menu[6].dp = key_accent1_upper_table;
   edit_menu[7].dp = key_accent1_shift_lower_table;
   edit_menu[8].dp = key_accent1_shift_upper_table;
   edit_menu[9].dp = key_accent2_lower_table;
   edit_menu[10].dp = key_accent2_upper_table;
   edit_menu[11].dp = key_accent2_shift_lower_table;
   edit_menu[12].dp = key_accent2_shift_upper_table;

   set_gfx_mode(GFX_AUTODETECT, 320, 240, 0, 0);
   set_palette(desktop_palette);

   do_dialog(main_dlg, -1);

   return 0;
}

