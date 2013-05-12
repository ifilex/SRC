typedef struct
{
  UWORD exSignature;
  UWORD exExtraBytes;
  UWORD exPages;
  UWORD exRelocItems;
  UWORD exHeaderSize;
  UWORD exMinAlloc;
  UWORD exMaxAlloc;
  UWORD exInitSS;
  UWORD exInitSP;
  UWORD exCheckSum;
  UWORD exInitIP;
  UWORD exInitCS;
  UWORD exRelocTable;
  UWORD exOverlay;
}
exe_header;

#define MAGIC 0x5a4d
