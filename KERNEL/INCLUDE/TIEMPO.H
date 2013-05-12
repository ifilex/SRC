#ifndef DOSC_TIME_H
#define DOSC_TIME_H
#define TM_HOUR(t)      (((t)>>11)&0x1f)
#define TM_MIN(t)       (((t)>>5)&0x3f)
#define TM_DEC(t)       ((t)&0x1f)

#define TM_ENCODE(h,m,d) ((((h&0x1f))<<11)|(((m)&0x3f)<<5)|((d)&0x1f))

typedef UWORD time;

#endif
