#ifndef DOSC_DATE_H
#define DOSC_DATE_H

#define DT_YEAR(d)      (((d)>>9)&0x7f)
#define DT_MONTH(d)     (((d)>>5)&0x0f)
#define DT_DAY(d)       ((d)&0x1f)

#define DT_ENCODE(d,m,y) (((d)&0x1f)|(((m)&0x0f)<<5)|(((y)&0x7f)<<9))

#define EPOCH_WEEKDAY   2       
#define EPOCH_MONTH     1       
#define EPOCH_DAY       1      
#define EPOCH_YEAR      1980    

typedef UWORD date;

#endif
