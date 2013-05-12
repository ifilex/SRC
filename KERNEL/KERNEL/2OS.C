#include "port.h"
#include "todo.h"

#ifdef PROTO
sft FAR *get_sft(COUNT);
#else
sft FAR *get_sft();
#endif


COUNT DosDevIOctl(iregs FAR * r, COUNT FAR * err)
{
  sft FAR *s;
  struct dpb FAR *dpbp;
  BYTE FAR *pBuffer = MK_FP(r->DS, r->DX);
  COUNT nMode;

  switch (r->AL)
  {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x06:
    case 0x07:
    case 0x0a:
    case 0x0c:

      if ((s = get_sft(r->BX)) == (sft FAR *) - 1)
      {
        *err = DE_INVLDHNDL;
        return 0;
      }
      break;

    case 0x04:
    case 0x05:
    case 0x08:
    case 0x09:
    case 0x0d:
    case 0x0e:
    case 0x0f:
    case 0x10:
    case 0x11:
      if (r->BL > nblkdev)
      {
        *err = DE_INVLDDRV;
        return 0;
      }
      else
      {
        if (r->BL == 0)
          dpbp = &blk_devices[default_drive];
        else
          dpbp = &blk_devices[r->BL - 1];
      }
      break;

    case 0x0b:
      break;

    default:
      *err = DE_INVLDFUNC;
      return 0;
  }

  switch (r->AL)
  {
    case 0x00:
      r->DX = r->AX = s->sft_flags;

      if ((s->sft_flags & SFT_FSHARED)
          || !(s->sft_flags & SFT_FDEVICE))
      {
        r->AH = 0;
      }
      break;

    case 0x01:
      if (!(s->sft_flags & SFT_FDEVICE))
      {
        *err = DE_INVLDFUNC;
        return 0;
      }

      r->AL = s->sft_flags_lo = SFT_FDEVICE | r->DL;
      break;

    case 0x0c:
      nMode = C_GENIOCTL;
      goto IoCharCommon;
    case 0x02:
      nMode = C_IOCTLIN;
      goto IoCharCommon;
    case 0x10:
      nMode = C_IOCTLQRY;
      goto IoCharCommon;
    case 0x03:
      nMode = C_IOCTLOUT;
    IoCharCommon:
      if (!(s->sft_flags & SFT_FDEVICE)
          || ((r->AL == 0x10) && !(s->sft_dev->dh_attr & ATTR_QRYIOCTL))
          || ((r->AL == 0x0c) && !(s->sft_dev->dh_attr & ATTR_GENIOCTL)))
      {
        if (s->sft_dev->dh_attr & SFT_FIOCTL)
        {
          CharReqHdr.r_unit = 0;
          CharReqHdr.r_length = sizeof(request);
          CharReqHdr.r_command = nMode;
          CharReqHdr.r_count = r->CX;
          CharReqHdr.r_trans = pBuffer;
          CharReqHdr.r_status = 0;
          execrh((request FAR *) & CharReqHdr,
                 s->sft_dev);
          if (CharReqHdr.r_status & S_ERROR)
            return char_error(&CharReqHdr,
                              s->sft_dev);
          if (r->AL == 0x07)
          {
            r->AL =
                CharReqHdr.r_status & S_BUSY ?
                00 : 0xff;
          }
          break;
        }
      }
      *err = DE_INVLDFUNC;
      return 0;

    case 0x0d:
      nMode = C_GENIOCTL;
      goto IoBlockCommon;
    case 0x04:
      nMode = C_IOCTLIN;
      goto IoBlockCommon;
    case 0x11:
      nMode = C_IOCTLQRY;
      goto IoBlockCommon;
    case 0x05:
      nMode = C_IOCTLOUT;
    IoBlockCommon:
      if (!(dpbp->dpb_device->dh_attr & ATTR_IOCTL)
      || ((r->AL == 0x11) && !(dpbp->dpb_device->dh_attr & ATTR_QRYIOCTL))
          || ((r->AL == 0x0d) && !(dpbp->dpb_device->dh_attr & ATTR_GENIOCTL)))
      {
        *err = DE_INVLDFUNC;
        return 0;
      }

      CharReqHdr.r_unit = r->BL;
      CharReqHdr.r_length = sizeof(request);
      CharReqHdr.r_command = nMode;
      CharReqHdr.r_count = r->CX;
      CharReqHdr.r_trans = pBuffer;
      CharReqHdr.r_status = 0;
      execrh((request FAR *) & CharReqHdr,
             dpbp->dpb_device);
      if (r->AL == 0x08)
      {
        if (CharReqHdr.r_status & S_ERROR)
        {
          *err = DE_DEVICE;
          return 0;
        }
        r->AX = (CharReqHdr.r_status & S_BUSY) ? 1 : 0;
      }
      else
      {
        if (CharReqHdr.r_status & S_ERROR)
        {
          *err = DE_DEVICE;
          return 0;
        }
      }
      break;

    case 0x06:
      if (s->sft_flags & SFT_FDEVICE)
      {
        r->AL = s->sft_flags & SFT_FEOF ? 0 : 0xFF;
      }
      else
        r->AL = s->sft_posit >= s->sft_size ? 0xFF : 0;
      break;

    case 0x07:
      if (s->sft_flags & SFT_FDEVICE)
      {
        goto IoCharCommon;
      }
      r->AL = 0;
      break;

    case 0x08:
      if (dpbp->dpb_device->dh_attr & ATTR_EXCALLS)
      {
        nMode = C_REMMEDIA;
        goto IoBlockCommon;
      }
      *err = DE_INVLDFUNC;
      return 0;

    case 0x09:
      r->DX = dpbp->dpb_device->dh_attr;
      break;

    case 0x0a:
      r->DX = s->sft_dcb->dpb_device->dh_attr;
      break;

    case 0x0e:
      nMode = C_GETLDEV;
      goto IoLogCommon;
    case 0x0f:
      nMode = C_SETLDEV;
    IoLogCommon:
      if (!(dpbp->dpb_device->dh_attr & ATTR_GENIOCTL))
      {
        if (r->BL == 0)
          r->BL = default_drive;

        CharReqHdr.r_unit = r->BL;
        CharReqHdr.r_length = sizeof(request);
        CharReqHdr.r_command = nMode;
        CharReqHdr.r_count = r->CX;
        CharReqHdr.r_trans = pBuffer;
        CharReqHdr.r_status = 0;
        execrh((request FAR *) & CharReqHdr,
               dpbp->dpb_device);

        if (CharReqHdr.r_status & S_ERROR)
          *err = DE_ACCESS;
        else
          *err = SUCCESS;
        return 0;
      }
      *err = DE_INVLDFUNC;
      return 0;

    default:
      *err = DE_INVLDFUNC;
      return 0;
  }
  *err = SUCCESS;
  return 0;
}
