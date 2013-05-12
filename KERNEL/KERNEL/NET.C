#include "port.h"
#include "todo.h"

UWORD get_machine_name(BYTE FAR * netname)
{
    BYTE FAR * xn;

    xn = MK_FP(net_name, 0);
    fbcopy((BYTE FAR *) netname, xn, 15);
    return (NetBios);

}

VOID set_machine_name(BYTE FAR * netname, UWORD name_num)
{
    BYTE FAR * xn;

    xn = MK_FP(net_name, 0);
    NetBios = name_num;
    fbcopy( xn, (BYTE FAR *) netname, 15);
    net_set_count++;

}

