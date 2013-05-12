#include "port.h"
#include "todo.h"
extern UWORD internalUpcase(UWORD c);

#ifdef __TURBOC__
void __int__(int);
void __emit__();
#endif


#if defined(_MSC_VER)
#define asm __asm
#pragma pack(1)
#elif defined(_QC) || defined(__WATCOM__)
#pragma pack(1)
#elif defined(__ZTC__)
#pragma ZTC align 1
#elif defined(__TURBOC__) && (__TURBOC__ > 0x202)
#pragma option -a-
#endif

struct ctryInfo
{                               
  WORD dateFmt;                
  char curr[5];                
  char thSep[2];               
  char point[2];         
  char dateSep[2];         
  char timeSep[2];            
  BYTE currFmt;               
  BYTE prescision;            
  BYTE timeFmt;               
    VOID(FAR * upCaseFct) (VOID);      
  char dataSep[2];             
};

struct _VectorTable
{
  VOID FAR *Table;
  BYTE FnCode;
};

struct _NlsInfo
{
  struct extCtryInfo
  {
    BYTE reserved[8];
    BYTE countryFname[64];
    WORD sysCodePage;
    WORD nFnEntries;
    struct _VectorTable VectorTable[6];

    WORD countryCode;           
    WORD codePage;           

    struct ctryInfo nlsCtryInfo;
  }
  nlsExtCtryInfo;

  char yesCharacter;
  char noCharacter;

  WORD upNCsize;                
  char upNormCh[128];

  WORD upFCsize;             
  char upFileCh[128];

  WORD collSize;               
  char collSeq[256];

  WORD dbcSize;           
  WORD dbcEndMarker;          

  struct chFileNames
  {
    WORD fnSize;           
    BYTE dummy1;
    char firstCh,
      lastCh;                 
    BYTE dummy2;
    char firstExcl,
      lastExcl;                
    BYTE dummy3;
    BYTE numSep;                
    char fnSeparators[14];
  }
  nlsFn;
}
nlsInfo
#ifdef INIT_NLS_049
=                               
#include "tecla001.inc"
#else
=                            
#include "tecla002.inc"
#endif
 ;

#define normalCh nlsInfo.upNormCh
#define fileCh nlsInfo.upFileCh
#define yesChar nlsInfo.yesCharacter
#define noChar nlsInfo.noCharacter

#define PathSep(c) ((c)=='/'||(c)=='\\')
#define DriveChar(c) (((c)>='A'&&(c)<='Z')||((c)>='a'&&(c)<='z'))

struct CpiHeader
{
  BYTE name[8];                
  BYTE reserved[8];
  WORD nPointers;              

  struct
  {
    BYTE pointerType;       
    DWORD offset;           
  }
  pointer[1];
};

struct CountryRecord
{
  WORD length;              
  WORD country;              
  WORD codePage;             
  WORD reserved[2];
  DWORD subCountryOffset;      
};

struct CountryTableDescr
{
  WORD length;               
  WORD id;                    
  DWORD offset;             
};


#if defined (_MSC_VER) || defined(_QC) || defined(__WATCOMC__)
#pragma pack()
#elif defined (__ZTC__)
#pragma ZTC align
#elif defined(__TURBOC__) && (__TURBOC__ > 0x202)
#pragma option -a.
#endif

COUNT NlsFuncInst(VOID)
{
  BYTE cNlsRet;

#ifndef __TURBOC__
  asm {
    xor bx,bx
    mov ax, 0x1400
    int 0x2F
    mov cNlsRet, al
  }
#else
  _BX = 0;
  _AX = 0x1400;
  __int__(0x2f);
  cNlsRet = _AL;
#endif

  return cNlsRet;
}

BOOL
GetGlblCodePage(UWORD FAR * ActvCodePage, UWORD FAR * SysCodePage)
{
  *ActvCodePage = nlsInfo.nlsExtCtryInfo.codePage;
  *SysCodePage = nlsInfo.nlsExtCtryInfo.sysCodePage;
  return TRUE;
}

BOOL
SetGlblCodePage(UWORD FAR * ActvCodePage, UWORD FAR * SysCodePage)
{
  nlsInfo.nlsExtCtryInfo.codePage = *ActvCodePage;
  nlsInfo.nlsExtCtryInfo.sysCodePage = *SysCodePage;
  return TRUE;
}

UWORD SetCtryInfo(UBYTE FAR * lpShrtCode, UWORD FAR * lpLongCode,
                  BYTE FAR * lpTable, UBYTE * nRetCode)
{
  UWORD CntryCode;
  UBYTE nNlsEntry;
  UWORD uSegTable,
    uOffTable;
  UBYTE nLclRet;

  if (0xff != *lpShrtCode)
    CntryCode = *lpShrtCode;
  else
    CntryCode = *lpLongCode;

  if (CntryCode == nlsInfo.nlsExtCtryInfo.countryCode)
  {
    *nRetCode = 0;
    return CntryCode;
  }

  if (NlsFuncInst() >= 0)
  {
    *nRetCode = 0xff;
    return 0xffff;
  }

  uSegTable = FP_SEG(lpTable);
  uOffTable = FP_OFF(lpTable);

#ifndef __TURBOC__
  asm {
    push ds
    mov bx, CntryCode
    mov ax, uSegTable
    mov dx, uOffTable
    mov ds, ax
    mov ax, 0x1404
    int 0x2F
    pop ds
    mov CntryCode, bx
    mov nLclRet, al
  }
#else
  _BX = CntryCode;
  _AX = uSegTable;
  _DX = uOffTable;
  _DS = _AX;
  _AX = 0x1404;
  __int__(0x2f);
  __emit__(0x1f); 
  CntryCode = _BX;
  nLclRet = _AL;
#endif
   *nRetCode = nLclRet;
  return CntryCode;
}

UWORD GetCtryInfo(UBYTE FAR * lpShrtCode, UWORD FAR * lpLongCode,
                  BYTE FAR * lpTable)
{
  fbcopy((BYTE FAR *) & nlsInfo.nlsExtCtryInfo.nlsCtryInfo,
         lpTable, sizeof(struct ctryInfo));
  return nlsInfo.nlsExtCtryInfo.countryCode;
}

BOOL ExtCtryInfo(UBYTE nOpCode, UWORD CodePageID, UWORD InfoSize, VOID FAR * Information)
{
  VOID FAR *lpSource;
  COUNT nIdx;

  if (0xffff != CodePageID)
  {
    UBYTE nNlsEntry;

    if (NlsFuncInst() >= 0)
      return FALSE;

#ifndef __TURBOC__
    asm {
      mov bp, word ptr nOpCode
      mov bx, CodePageID
      mov si, word ptr Information + 2
      mov ds, si
      mov si, word ptr Information
      mov ax, 0x1402
      int 0x2F
      cmp al, 0
      mov nNlsEntry, al
    }
#else
    __emit__(0x1e, 0x55, 0x56); 
    _BX = CodePageID;
    _SI = ((WORD *)&Information)[1];
    _DS = _SI;
    _SI = *(WORD *)&Information;
    _BP = *(WORD *)&nOpCode;
    _BP &= 0x00ff;
    _AX = 0x1402;
    __int__(0x2f);
    nNlsEntry = _AL;
    __emit__(0x5e, 0x5d, 0x1f); /* pop si; pop bp; pop ds */
#endif

    if (0 != nNlsEntry)
        return FALSE;

    return TRUE;
  }

  CodePageID = nlsInfo.nlsExtCtryInfo.codePage;

  for (nIdx = 0; nIdx < nlsInfo.nlsExtCtryInfo.nFnEntries; nIdx++)
  {
    if (nlsInfo.nlsExtCtryInfo.VectorTable[nIdx].FnCode == nOpCode)
    {
      BYTE FAR *bp = Information;
      lpSource = nlsInfo.nlsExtCtryInfo.VectorTable[nIdx].Table;

      if (nOpCode == 1)
      {
        bp++;                   

        *bp = (BYTE) (sizeof(struct ctryInfo) + 4);
        bp += 2;

        fbcopy(lpSource, bp, InfoSize > 3 ? InfoSize - 3 : 0);
      }
      else
      {
        *bp++ = nOpCode;
        *((VOID FAR **) bp) = lpSource;
      }
      return TRUE;
    }
  }

  return FALSE;
}

UWORD internalUpcase(UWORD c)
{
  if (!(c & 0x80))
    return c;

  return (c & 0xff00) | (nlsInfo.upNormCh[c & 0x7f] & 0xff);
}

char upMChar(UPMAP map, char ch)
{
  return (ch >= 'a' && ch <= 'z') ? ch + 'A' - 'a' :
      ((unsigned)ch > 0x7f ? map[ch & 0x7f] : ch);
}

VOID upMMem(UPMAP map, char FAR * str, unsigned len)
{
  REG unsigned c;

  if (len)
    do
    {
      if ((c = *str) >= 'a' && c <= 'z')
        *str += 'A' - 'a';
      else if (c > 0x7f)
        *str = map[c & 0x7f];
      ++str;
    }
    while (--len);
}

BYTE yesNo(char ch)            
{
  ch = upMChar(normalCh, ch);
  if (ch == noChar)
    return 0;
  if (ch == yesChar)
    return 1;
  return 2;
}

char upChar(char ch)           
{
  return upMChar(normalCh, ch);
}

VOID upString(char FAR * str)  
{
  upMMem(normalCh, str, fstrlen(str));
}

VOID upMem(char FAR * str, unsigned len)    
{
  upMMem(normalCh, str, len);
}

char upFChar(char ch)          
{
  return upMChar(fileCh, ch);
}

VOID upFString(char FAR * str) 
{
  upMMem(fileCh, str, fstrlen(str));
}

VOID upFMem(char FAR * str, unsigned len)     
{
  upMMem(fileCh, str, len);
}


static BOOL ReadCountryTable(COUNT file, WORD id, ULONG offset)
{
  VOID *buf;               
  UWORD maxSize;              
  UWORD length;               
  BOOL rc = TRUE;

  switch (id)
  {
    case 1:                    
      buf = &nlsInfo.nlsExtCtryInfo.countryCode;
      maxSize = sizeof(struct ctryInfo) + sizeof(WORD) * 2;
      break;

    case 2:                   
      buf = &normalCh[0];
      maxSize = sizeof normalCh;
      break;

    case 4:                   
      buf = &fileCh[0];
      maxSize = sizeof fileCh;
      break;

    case 5:                  
      buf = &nlsInfo.nlsFn.dummy1;
      maxSize = sizeof(struct chFileNames) - sizeof(WORD);
      break;

    case 6:                 
      buf = &nlsInfo.collSeq[0];
      maxSize = sizeof nlsInfo.collSeq;
      break;

    default:               
      buf = 0;
      break;
  }

  if (buf)
  {
    dos_lseek(file, offset, 0);
    dos_read(file, &length, sizeof(length));

    if (length > maxSize)
      length = maxSize;

    if (dos_read(file, buf, length) != length)
      rc = FALSE;

    if (id == 1)
      nlsInfo.nlsExtCtryInfo.nlsCtryInfo.upCaseFct = CharMapSrvc;
  }

  return rc;
}



BOOL FAR LoadCountryInfo(char FAR * filename, WORD ctryCode, WORD codePage)
{
  struct CpiHeader hdr;
  struct CountryRecord ctry;
  struct CountryTableDescr ct;
  COUNT i,
    nCountries,
    nSubEntries;
  ULONG currpos;
  int rc = FALSE;
  COUNT file;

  if ((file = dos_open(filename, 0)) < 0)
    return rc;

  if (dos_read(file, &hdr, sizeof(hdr)) == sizeof(hdr))
  {
    if (!fstrncmp(hdr.name, "\377COUNTRY", 8))
    {
      dos_lseek(file, hdr.pointer[0].offset, 0);
      dos_read(file, &nCountries, sizeof(nCountries));

      for (i = 0; i < nCountries; i++)
      {
        if (dos_read(file, &ctry, sizeof(ctry)) != sizeof(ctry))
          break;

        if (ctry.country == ctryCode && (!codePage || ctry.codePage == codePage))
        {
          dos_lseek(file, ctry.subCountryOffset, 0);
          dos_read(file, &nSubEntries, sizeof(nSubEntries));
          currpos = ctry.subCountryOffset + sizeof(nSubEntries);

          for (i = 0; i < nSubEntries; i++)
          {
            dos_lseek(file, currpos, 0);
            if (dos_read(file, &ct, sizeof(ct)) != sizeof(ct))
              break;

            currpos += ct.length + sizeof(ct.length);
            ReadCountryTable(file, ct.id, ct.offset + 8);
          }

          if (i == nSubEntries)
            rc = TRUE;

          break;
        }
      }
    }
  }
  dos_close(file);
  return rc;
}
