#include "port.h"
#include "todo.h"

VOID mcb_init();
VOID mcb_print();
VOID show_chain();

#define nxtMCBsize(mcb,size)	\
	MK_FP(far2para((VOID FAR *) (mcb)) + (size) + 1, 0)
#define nxtMCB(mcb) nxtMCBsize((mcb), (mcb)->m_size)

#define mcbFree(mcb) ((mcb)->m_psp == FREE_PSP)
#define mcbValid(mcb)			\
	((mcb)->m_type == MCB_NORMAL || (mcb)->m_type == MCB_LAST)

#define para2far(seg) (mcb FAR *)MK_FP((seg) , 0)

static COUNT joinMCBs(mcb FAR *p)
{	mcb FAR *q;

	while(p->m_type == MCB_NORMAL && mcbFree(q = nxtMCB(p))) {
		if(!mcbValid(q))
			return DE_MCBDESTRY;
                p->m_type = q->m_type;         
                p->m_size += q->m_size + 1;    
                q->m_type = 'K';                     
	}

	return SUCCESS;
}


seg far2para(VOID FAR * p)
{
  return FP_SEG(p) + (FP_OFF(p) >> 4);
}

seg long2para(LONG size)
{
  return ((size + 0x0f) >> 4);
}

VOID FAR *add_far(VOID FAR * fp, ULONG off)
{
	off += FP_OFF(fp);

	return MK_FP(FP_SEG(fp) + (UWORD)(off >> 4), (UWORD)off & 0xf);
}

VOID FAR *adjust_far(VOID FAR * fp)
{
  return MK_FP(FP_SEG(fp) + (FP_OFF(fp) >> 4), FP_OFF(fp) & 0xf);
}

#undef REG
#define REG

#ifdef KERNEL
COUNT DosMemAlloc(UWORD size, COUNT mode, seg FAR * para, UWORD FAR * asize)
{
  REG mcb FAR *p;
  mcb FAR * foundSeg;
  mcb FAR * biggestSeg;

  p = para2far(first_mcb);
  biggestSeg = foundSeg = NULL;

  FOREVER
  {
    if (!mcbValid(p))
      return DE_MCBDESTRY;

    if(mcbFree(p)) {   
        if(joinMCBs(p) != SUCCESS)    
                  return DE_MCBDESTRY;      

		if(!biggestSeg || biggestSeg->m_size < p->m_size)
			biggestSeg = p;

                if(p->m_size >= size) {        
			switch(mode) {
                                case LAST_FIT:         
				default:
					foundSeg = p;
					break;

                                case LARGEST:         
                                                                     
					break;

                                case BEST_FIT:          
					if(!foundSeg || foundSeg->m_size > p->m_size)
						foundSeg = p;
					break;

                                case FIRST_FIT:         
					foundSeg = p;
                                        goto stopIt;   
			}
		}
	 }

	 if(p->m_type == MCB_LAST)
                break;        

        p = nxtMCB(p);
  }

  if(mode == LARGEST && biggestSeg && biggestSeg->m_size >= size)
  	*asize = (foundSeg = biggestSeg)->m_size;

  if(!foundSeg || !foundSeg->m_size) { 
   	if(asize)
   		*asize = biggestSeg? biggestSeg->m_size: 0;
  	return DE_NOMEM;
  }

stopIt:             

	if(mode != LARGEST && size != foundSeg->m_size) {
		if(mode == LAST_FIT) {
			p = foundSeg;
                        p->m_size -= size + 1;
			foundSeg = nxtMCB(p);

			foundSeg->m_type = p->m_type;
			p->m_type = MCB_NORMAL;
		}
                else {        
			p = nxtMCBsize(foundSeg, size);
			p->m_size = foundSeg->m_size - size - 1;

			p->m_type = foundSeg->m_type;
			foundSeg->m_type = MCB_NORMAL;
		}

                p->m_psp = FREE_PSP;          

		foundSeg->m_size = size;
	}

        foundSeg->m_psp = cu_psp;      
	foundSeg->m_name[0] = '\0';

    *para = far2para((VOID FAR *) (BYTE FAR *) foundSeg);
    return SUCCESS;
}

COUNT FAR init_call_DosMemAlloc(UWORD size, COUNT mode, seg FAR *para, UWORD FAR *asize)
{
  return DosMemAlloc(size, mode, para, asize);
}

COUNT DosMemLargest(UWORD FAR * size)
{
  REG mcb FAR *p;
  mcb FAR *q;
  COUNT found;

  p = para2far(first_mcb);

        *size = 0;             
	FOREVER {
                if(!mcbValid(p))                      
			return DE_MCBDESTRY;

                if(mcbFree(p)) {      
			if(joinMCBs(p) != SUCCESS)
				return DE_MCBDESTRY;

			if(*size < p->m_size)
				*size = p->m_size;
		}

                if(p->m_type == MCB_LAST)        
			break;
		p = nxtMCB(p);
	}


  return *size? SUCCESS: DE_NOMEM;
}

COUNT DosMemFree(UWORD para)
{
  REG mcb FAR *p;
  COUNT i;

  if(!para)                           
    return DE_INVLDMCB;

  p = para2far(para);

  if (!mcbValid(p))
    return DE_INVLDMCB;

  p->m_psp = FREE_PSP;
  for (i = 0; i < 8; i++)
    p->m_name[i] = '\0';

#if 0
  for (p = (mcb FAR *) (MK_FP(first_mcb, 0)); p->m_type != MCB_LAST; p = q)
  {
    q = nxtMCB(p);
    if (q->m_type != MCB_NORMAL && q->m_type != MCB_LAST)
      return DE_MCBDESTRY;
    if (p->m_psp != FREE_PSP)
      continue;

    if (q->m_psp == FREE_PSP)
    {
      p->m_type = q->m_type;
      p->m_size += q->m_size + 1;
      q = p;
    }
  }
#endif
  return SUCCESS;
}

COUNT DosMemChange(UWORD para, UWORD size, UWORD * maxSize)
{
  REG mcb FAR *p,
    FAR * q;
  REG COUNT i;

  p = para2far(para - 1);            

  if (!mcbValid(p))
    return DE_MCBDESTRY;

  if (size > p->m_size)
  {
  	if(joinMCBs(p) != SUCCESS)
  		return DE_MCBDESTRY;

        if(size > p->m_size) {        
		if(maxSize)
			*maxSize = p->m_size;
		return DE_NOMEM;
	}
  }

  if (size < p->m_size)
  {
    q = nxtMCBsize(p, size);
    q->m_type = p->m_type;
    q->m_size = p->m_size - size - 1;

    p->m_size = size;

    p->m_type = MCB_NORMAL;

    q->m_psp = FREE_PSP;
    for (i = 0; i < 8; i++)
      q->m_name[i] = '\0';
  }
  return SUCCESS;
}

COUNT DosMemCheck(void)
{
  REG mcb FAR *p;

  p = para2far(first_mcb);

  while(p->m_type != MCB_LAST) 
  {
    if (p->m_type != MCB_NORMAL)
      return DE_MCBDESTRY;

      p = nxtMCB(p);
  }

  return SUCCESS;
}

COUNT FreeProcessMem(UWORD ps)
{
  mcb FAR *p;

  p = para2far(first_mcb);

  while(mcbValid(p))                  
  {
    if (p->m_psp == ps)
      DosMemFree(FP_SEG(p));

    if (p->m_type == MCB_LAST)
	  return SUCCESS;

    p = nxtMCB(p);
  }

  return DE_MCBDESTRY;
}

#if 0
COUNT DosGetLargestBlock(UWORD FAR * block)
{
  UWORD sz = 0;
  mcb FAR *p;
  *block = sz;

  p = (mcb FAR *) (MK_FP(first_mcb, 0));

  for (;;)
  {
    if (p->m_type != MCB_NORMAL && p->m_type != MCB_LAST)
      return DE_MCBDESTRY;

    if (p->m_psp == FREE_PSP && p->m_size > sz)
      sz = p->m_size;

    if (p->m_type == MCB_LAST)
      break;
    p = nxtMCB(p);

  }
  *block = sz;
  return SUCCESS;
}
#endif

VOID show_chain(void)
{
  mcb FAR *p = para2far(first_mcb);

  for (;;)
  {
    mcb_print(p);
    if (p->m_type == MCB_LAST || p->m_type != MCB_NORMAL)
      return;
    else
      p = nxtMCB(p);
  }
}

VOID mcb_print(mcb FAR * mcbp)
{
  static BYTE buff[9];
  VOID _fmemcpy();

  _fmemcpy((BYTE FAR *) buff, (BYTE FAR *) (mcbp->m_name), 8);
  buff[8] = '\0';
  printf("%04x-%04x -> |%s| m_type = 0x%02x '%c'; m_psp = 0x%04x; m_size = 0x%04x\n",
         FP_SEG(mcbp),
         FP_OFF(mcbp),
         *buff == '\0' ? "*NO-ID*" : buff,
         mcbp->m_type, mcbp->m_type > ' '? mcbp->m_type: ' ',
         mcbp->m_psp,
         mcbp->m_size);
}

VOID _fmemcpy(BYTE FAR * d, BYTE FAR * s, REG COUNT n)
{
  while (n--)
    *d++ = *s++;

}
#endif
