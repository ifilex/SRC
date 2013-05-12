#include "port.h"
#include "todo.h"

static BYTE *con_name = "CON";

#ifdef PROTO
VOID kbfill(keyboard FAR *, UCOUNT, BOOL, UWORD *);
struct dhdr FAR *finddev(UWORD attr_mask);

#else
VOID kbfill();
struct dhdr FAR *finddev();
#endif


struct dhdr FAR *finddev(UWORD attr_mask)
{
  struct dhdr far *dh;

  for (dh = nul_dev.dh_next; FP_OFF(dh) != 0xFFFF; dh = dh->dh_next)
  {
    if (dh->dh_attr & attr_mask)
      return dh;
  }

 
  return &nul_dev;
}

VOID cso(COUNT c)
{
  BYTE buf = c;
  struct dhdr FAR *lpDevice;

  CharReqHdr.r_length = sizeof(request);
  CharReqHdr.r_command = C_OUTPUT;
  CharReqHdr.r_count = 1;
  CharReqHdr.r_trans = (BYTE FAR *) (&buf);
  CharReqHdr.r_status = 0;
  execrh((request FAR *) & CharReqHdr,
         lpDevice = (struct dhdr FAR *)finddev(ATTR_CONOUT));
  if (CharReqHdr.r_status & S_ERROR)
    char_error(&CharReqHdr, lpDevice);
}

VOID sto(COUNT c)
{
  static COUNT scratch; 

  DosWrite(STDOUT, 1, (BYTE FAR *)&c, (COUNT FAR *)scratch);
}

VOID mod_sto(REG UCOUNT c)
{
  if (c < ' ' && c != HT)
  {
    sto('^');
    sto(c + '@');
  }
  else
    sto(c);
}

VOID destr_bs(void)
{
  sto(BS);
  sto(' ');
  sto(BS);
}


VOID Do_DosIdle_loop(void)
{
    FOREVER{
        if(StdinBusy())
            return;
        else
        {
            DosIdle_int();
            continue;
        }
    }
}


UCOUNT _sti(void)
{
  static COUNT scratch;
  UBYTE c;
  while (GenericRead(STDIN, 1, (BYTE FAR *)&c, (COUNT FAR *)&scratch, TRUE)
    != 1);
  return c;
}

BOOL con_break(void)
{
  CharReqHdr.r_unit = 0;
  CharReqHdr.r_status = 0;
  CharReqHdr.r_command = C_NDREAD;
  CharReqHdr.r_length = sizeof(request);
  execrh((request FAR *) & CharReqHdr, (struct dhdr FAR *)finddev(ATTR_CONIN));
  if (CharReqHdr.r_status & S_BUSY)
    return FALSE;
  if (CharReqHdr.r_ndbyte == CTL_C)
  {
    _sti();
    return TRUE;
  }
  else
    return FALSE;
}

BOOL StdinBusy(void)
{
  sft FAR *s;

  if ((s = get_sft(STDIN)) == (sft FAR *)-1)
    return FALSE; 
  if (s->sft_count == 0 || (s->sft_mode & SFT_MWRITE))
    return FALSE; 
  if (s->sft_flags & SFT_FDEVICE)
  {
    CharReqHdr.r_unit = 0;
    CharReqHdr.r_status = 0;
    CharReqHdr.r_command = C_ISTAT;
    CharReqHdr.r_length = sizeof(request);
    execrh((request FAR *) & CharReqHdr, s->sft_dev);
    if (CharReqHdr.r_status & S_BUSY)
      return TRUE;
    else
      return FALSE;
  }
  else
    return FALSE; 
}

VOID KbdFlush(void)
{
  CharReqHdr.r_unit = 0;
  CharReqHdr.r_status = 0;
  CharReqHdr.r_command = C_IFLUSH;
  CharReqHdr.r_length = sizeof(request);
  execrh((request FAR *) & CharReqHdr, (struct dhdr FAR *)finddev(ATTR_CONIN));
}

static VOID kbfill(keyboard FAR * kp, UCOUNT c, BOOL ctlf, UWORD *vp)
{
  if (kp->kb_count > kp->kb_size)
  {
    sto(BELL);
    return;
  }
  kp->kb_buf[kp->kb_count++] = c;
  if (!ctlf)
  {
    mod_sto(c);
    *vp += 2;
  }
  else
  {
    sto(c);
    if (c != HT)
      ++*vp;
    else
      *vp = (*vp + 8) & -8;
  }
}

VOID sti(keyboard FAR * kp)
{
  REG UWORD c,
    cu_pos = scr_pos;
  UWORD
    virt_pos = scr_pos;
  WORD init_count = kp->kb_count;
#ifndef NOSPCL
  static BYTE local_buffer[LINESIZE];
#endif

  if (kp->kb_size == 0)
    return;
  if (kp->kb_size <= kp->kb_count || kp->kb_buf[kp->kb_count] != CR)
    kp->kb_count = 0;
  FOREVER
  {

   Do_DosIdle_loop();

    switch (c = _sti())
    {
    	case CTL_C:
    		handle_break();
      case CTL_F:
        continue;

#ifndef NOSPCL
      case SPCL:
        switch (c = _sti())
        {
          case LEFT:
            goto backspace;

          case F3:
            {
              REG COUNT i;

              for (i = kp->kb_count; local_buffer[i] != '\0'; i++)
              {
                c = local_buffer[kp->kb_count];
                if (c == '\r' || c == '\n')
                  break;
                kbfill(kp, c, FALSE, &virt_pos);
              }
              break;
            }

          case RIGHT:
            c = local_buffer[kp->kb_count];
            if (c == '\r' || c == '\n')
              break;
            kbfill(kp, c, FALSE, &virt_pos);
            break;
        }
        break;
#endif

      case CTL_BS:
      case BS:
      backspace:
        if (kp->kb_count > 0)
        {
          if (kp->kb_buf[kp->kb_count - 1] >= ' ')
          {
            destr_bs();
	    --virt_pos;
          }
          else if ((kp->kb_buf[kp->kb_count - 1] < ' ')
                   && (kp->kb_buf[kp->kb_count - 1] != HT))
          {
            destr_bs();
            destr_bs();
	    virt_pos -= 2;
          }
          else if (kp->kb_buf[kp->kb_count - 1] == HT)
          {
            do
            {
              destr_bs();
	      --virt_pos;
            }
            while ((virt_pos > cu_pos) && (virt_pos & 7));
          }
          --kp->kb_count;
        }
        break;

      case CR:
        kbfill(kp, CR, TRUE, &virt_pos);
        kbfill(kp, LF, TRUE, &virt_pos);
#ifndef NOSPCL
        fbcopy((BYTE FAR *) kp->kb_buf,
               (BYTE FAR *) local_buffer, (COUNT) kp->kb_count);
        local_buffer[kp->kb_count] = '\0';
#endif
        return;

      case LF:
        sto(CR);
        sto(LF);
        break;

      case ESC:
        sto('\\');
        sto(CR);
        sto(LF);
        for (c = 0; c < cu_pos; c++)
          sto(' ');
        kp->kb_count = init_count;
        break;

      default:
        kbfill(kp, c, FALSE, &virt_pos);
        break;
    }
  }
}

VOID FAR init_call_sti(keyboard FAR * kp)
{
  sti(kp);
}
