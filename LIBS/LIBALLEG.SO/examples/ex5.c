/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program demonstrates how to access the keyboard.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


int main()
{
   int k;

   allegro_init();
   install_keyboard(); 

   /* keyboard input can be accessed with the readkey() function */
   printf("\nPress some keys (ESC to finish)\n");
   do {
      k = readkey();
      printf("readkey() returned %-6d (0x%04X)\n", k, k);
   } while ((k & 0xFF) != 27);

   /* the ASCII code is in the low byte of the return value */
   printf("\nPress some more keys (ESC to finish)\n");
   do {
      k = readkey();
      printf("ASCII code is %d\n", k & 0xFF);
   } while ((k & 0xFF) != 27);

   /* the hardware scancode is in the high byte of the return value */
   printf("\nPress some more keys (ESC to finish)\n");
   do {
      k = readkey();
      printf("Scancode is %d\n", k >> 8);
   } while ((k & 0xFF) != 27);

   /* key qualifiers */
   printf("\nPress some more keys (ESC to finish)\n");
   do {
      k = readkey();
      printf("You pressed ");
      if (key_shifts & KB_SHIFT_FLAG) printf("shift ");
      if (key_shifts & KB_CTRL_FLAG) printf("ctrl ");
      if (key_shifts & KB_ALT_FLAG) printf("alt ");
      if (key_shifts & KB_LWIN_FLAG) printf("lwin ");
      if (key_shifts & KB_RWIN_FLAG) printf("rwin ");
      if (key_shifts & KB_MENU_FLAG) printf("menu ");
      printf("'%c' [0x%02x]", (k & 0xFF), (k & 0xFF));
      if (key_shifts & KB_CAPSLOCK_FLAG) printf(" caps");
      if (key_shifts & KB_NUMLOCK_FLAG) printf(" num");
      if (key_shifts & KB_SCROLOCK_FLAG) printf(" scrl");
      printf("\n");
   } while ((k & 0xFF) != 27);

   /* various scancodes are defined in allegro.h as KEY_* constants */
   printf("\nPress F6\n");
   k = readkey();
   while ((k >> 8) != KEY_F6) {
      printf("Wrong key, stupid! I said press F6\n");
      k = readkey();
   }
   printf("Thank you\n");

   /* for detecting multiple simultaneous keypresses, use the key[] array */
   printf("\nPress a combination of numbers (ESC to finish)\n");
   do {
      if (key[KEY_0]) printf("0"); else printf(" ");
      if (key[KEY_1]) printf("1"); else printf(" ");
      if (key[KEY_2]) printf("2"); else printf(" ");
      if (key[KEY_3]) printf("3"); else printf(" ");
      if (key[KEY_4]) printf("4"); else printf(" ");
      if (key[KEY_5]) printf("5"); else printf(" ");
      if (key[KEY_6]) printf("6"); else printf(" ");
      if (key[KEY_7]) printf("7"); else printf(" ");
      if (key[KEY_8]) printf("8"); else printf(" ");
      if (key[KEY_9]) printf("9"); else printf(" ");
      printf("\r");
      fflush(stdout);
   } while (!key[KEY_ESC]);

   printf("\n");
   clear_keybuf();
   return 0;
}
