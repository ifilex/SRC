#include "port.h" 
#include "todo.h"

extern void spawn_int23(void);

#define CB_FLG *(UBYTE FAR*)MK_FP(0x40, 0x71)
#define CB_MSK 0x80

int control_break(void)
{	return (CB_FLG & CB_MSK) || con_break();
}

void handle_break(void)
{       CB_FLG &= ~CB_MSK;            
        KbdFlush();                            
        if(!ErrorMode)          
                if(InDOS) --InDOS;         

        spawn_int23();              
}
