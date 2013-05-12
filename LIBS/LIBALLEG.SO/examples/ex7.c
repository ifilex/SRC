/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program demonstrates how to use the timer routines.
 *    These can be a bit of a pain, because you have to be sure you lock 
 *    all the memory that is used inside your interrupt handlers.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


/* these must be declared volatile so the optimiser doesn't mess up */
volatile int x = 0;
volatile int y = 0;
volatile int z = 0;



/* timer interrupt handler */
void inc_x()
{
   x++;
}

END_OF_FUNCTION(inc_x);



/* timer interrupt handler */
void inc_y()
{
   y++;
}

END_OF_FUNCTION(inc_y);



/* timer interrupt handler */
void inc_z()
{
   z++;
}

END_OF_FUNCTION(inc_z);



int main()
{
   int c;

   allegro_init();
   install_keyboard(); 
   install_timer();

   /* use rest() to delay for a specified number of milliseconds */
   printf("\nTiming five seconds:\n");
   for (c=1; c<=5; c++) {
      printf("%d\n", c);
      rest(1000);
   }

   printf("\nPress a key to set up interrupt handlers\n");
   readkey();

   /* all variables and code used inside interrupt handlers must be locked */
   LOCK_VARIABLE(x);
   LOCK_VARIABLE(y);
   LOCK_VARIABLE(z);
   LOCK_FUNCTION(inc_x);
   LOCK_FUNCTION(inc_y);
   LOCK_FUNCTION(inc_z);

   /* the speed can be specified in milliseconds (this is once a second) */
   install_int(inc_x, 1000);

   /* or in beats per second (this is 10 ticks a second) */
   install_int_ex(inc_y, BPS_TO_TIMER(10));

   /* or in seconds (this is 10 ticks a second) */
   install_int_ex(inc_z, SECS_TO_TIMER(10));

   /* the interrupts are now active... */
   while (!keypressed())
      printf("x=%d, y=%d, z=%d\n", x, y, z);

   return 0;
}
