#ifndef LINESIZE
#define LINESIZE       127
#endif

typedef struct
{
  UBYTE ctCount;               
  BYTE ctBuffer[LINESIZE];     
}
CommandTail;
