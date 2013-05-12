#define MAX_CDSPATH 67

struct cds
{
  BYTE cdsCurrentPath[MAX_CDSPATH];
  UWORD cdsFlags;
  struct dpb FAR *
    cdsDpb;

  union
  {
    WORD _cdsStrtClst;
    struct
    {
      BYTE FAR *
        _cdsRedirRec;
      UWORD _cdsParam;
    }
    _cdsRedir;
  }
  _cdsUnion;

  WORD cdsJoinOffset;

};

#define cdsStrtClst _cdsUnion._csdStrtClst

#define CDSNETWDRV      0x8000
#define CDSPHYSDRV      0x4000
#define CDSJOINED       0x2000
#define CDSSUBST        0x1000
