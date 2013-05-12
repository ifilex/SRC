#include "port.h"
#include "todo.h"

#define PathSep(c) ((c)=='/'||(c)=='\\')
#define DriveChar(c) (((c)>='A'&&(c)<='Z')||((c)>='a'&&(c)<='z'))
#define DirChar(c) (!strchr("\"[]:|<>+=;,", (c)))
#define NameChar(c) (!strchr(".\"/\\[]:|<>+=;,*?", (c)))
#define WildChar(c) (!strchr(".\"/\\[]:|<>+=;,", (c)))

VOID XlateLcase(BYTE * szFname, COUNT nChars);
VOID DosTrimPath(BYTE FAR * lpszPathNamep);

VOID XlateLcase(BYTE * szFname, COUNT nChars)
{
  while (nChars--)
  {
    if (*szFname >= 'a' && *szFname <= 'z')
      *szFname -= ('a' - 'A');
    ++szFname;
  }
}

VOID SpacePad(BYTE * szString, COUNT nChars)
{
  REG COUNT i;

  for (i = strlen(szString); i < nChars; i++)
    szString[i] = ' ';
}

COUNT ParseDosName(BYTE FAR * lpszFileName,
                   COUNT * pnDrive,
                   BYTE * pszDir,
                   BYTE * pszFile,
                   BYTE * pszExt,
		   BOOL bAllowWildcards)
{
  COUNT nDirCnt,
    nFileCnt,
    nExtCnt;
  BYTE FAR *lpszLclDir,
    FAR * lpszLclFile,
    FAR * lpszLclExt;

  if (pszDir)
    *pszDir = '\0';
  if (pszFile)
    *pszFile = '\0';
  if (pszExt)
    *pszExt = '\0';
  lpszLclFile = lpszLclExt = lpszLclDir = 0;
  nDirCnt = nFileCnt = nExtCnt = 0;

  if (DriveChar(*lpszFileName) && ':' == lpszFileName[1])
  {
    if (pnDrive)
    {
      *pnDrive = *lpszFileName - 'A';
      if (*pnDrive > 26)
        *pnDrive -= ('a' - 'A');
    }
    lpszFileName += 2;
  }
  else
  {
    if (pnDrive)
    {
      *pnDrive = -1;
    }
  }
  if (!pszDir && !pszFile && !pszExt)
    return SUCCESS;

  lpszLclDir = lpszLclFile = lpszFileName;
  while (DirChar(*lpszFileName)) {
    if (PathSep(*lpszFileName))
      lpszLclFile = lpszFileName + 1;
    ++lpszFileName;
  }
  nDirCnt = lpszLclFile - lpszLclDir;

  lpszFileName = lpszLclFile;
  while (bAllowWildcards ? WildChar(*lpszFileName) : NameChar(*lpszFileName))
  {
    ++nFileCnt;
    ++lpszFileName;
  }
  if (nFileCnt == 0)
    return DE_FILENOTFND;

  lpszLclExt = lpszFileName;
  if ('.' == *lpszFileName)
  {
    lpszLclExt = ++lpszFileName;
    while (*lpszFileName)
    {
      if (bAllowWildcards ? WildChar(*lpszFileName) : NameChar(*lpszFileName))
      {
	++nExtCnt;
	++lpszFileName;
      }
      else
	return DE_FILENOTFND;
    }
  }
  else if (*lpszFileName)
    return DE_FILENOTFND;

  if (nDirCnt > PARSE_MAX)
    nDirCnt = PARSE_MAX;
  if (nFileCnt > FNAME_SIZE)
    nFileCnt = FNAME_SIZE;
  if (nExtCnt > FEXT_SIZE)
    nExtCnt = FEXT_SIZE;

  if (pszDir)
  {
    fbcopy(lpszLclDir, (BYTE FAR *) pszDir, nDirCnt);
    pszDir[nDirCnt] = '\0';
  }
  if (pszFile)
  {
    fbcopy(lpszLclFile, (BYTE FAR *) pszFile, nFileCnt);
    pszFile[nFileCnt] = '\0';
  }
  if (pszExt)
  {
    fbcopy(lpszLclExt, (BYTE FAR *) pszExt, nExtCnt);
    pszExt[nExtCnt] = '\0';
  }

  if (pszDir)
    DosTrimPath(pszDir);

  return SUCCESS;
}

COUNT ParseDosPath(BYTE FAR * lpszFileName,
                   COUNT * pnDrive,
                   BYTE * pszDir,
                   BYTE * pszCurPath)
{
  COUNT nDirCnt,
    nPathCnt;
  BYTE FAR *lpszLclDir,
   *pszBase = pszDir;

  *pszDir = '\0';
  lpszLclDir = 0;
  nDirCnt = nPathCnt = 0;

  if (DriveChar(*lpszFileName) && ':' == lpszFileName[1])
  {
    if (pnDrive)
    {
      *pnDrive = *lpszFileName - 'A';
      if (*pnDrive > 26)
        *pnDrive -= ('a' - 'A');
    }
    lpszFileName += 2;
  }
  else
  {
    if (pnDrive)
    {
      *pnDrive = -1;
    }
  }

  lpszLclDir = lpszFileName;
  if (!PathSep(*lpszLclDir))
  {
    strncpy(pszDir, pszCurPath, PARSE_MAX);
    nPathCnt = strlen(pszCurPath);
    if (!PathSep(pszDir[nPathCnt - 1]) && nPathCnt < PARSE_MAX)
      pszDir[nPathCnt++] = '\\';
    if (nPathCnt > PARSE_MAX)
      nPathCnt = PARSE_MAX;
    pszDir += nPathCnt;
  }

  while (NameChar(*lpszFileName)
         || PathSep(*lpszFileName)
         || '.' == *lpszFileName)
  {
    ++nDirCnt;
    ++lpszFileName;
  }

  if ((nDirCnt + nPathCnt) > PARSE_MAX)
    nDirCnt = PARSE_MAX - nPathCnt;

  if (pszDir)
  {
    fbcopy(lpszLclDir, (BYTE FAR *) pszDir, nDirCnt);
    pszDir[nDirCnt] = '\0';
  }

  DosTrimPath((BYTE FAR *) pszBase);

  nPathCnt = strlen(pszBase);
  if (2 == nPathCnt)            
  {
    if (!strcmp(pszBase, "\\."))
      pszBase[1] = '\0';
  }
  else if (2 < nPathCnt)
  {
    if (!strcmp(&pszBase[nPathCnt - 2], "\\."))
      pszBase[nPathCnt - 2] = '\0';
  }

  return SUCCESS;
}

BOOL IsDevice(BYTE * pszFileName)
{
  REG struct dhdr FAR *dhp = (struct dhdr FAR *)&nul_dev;
  BYTE szName[FNAME_SIZE];

  if (ParseDosName((BYTE FAR *) pszFileName,
                   (COUNT *) 0, TempBuffer, szName, (BYTE *) 0, FALSE)
      != SUCCESS)
    return FALSE;
  SpacePad(szName, FNAME_SIZE);

  if ((strcmp(szName, "/dev") == 0)  /* directorio de trabajo/intercambio */
      || (strcmp(szName, "\\dev") == 0))
    return TRUE;

  for (; -1l != (LONG) dhp; dhp = dhp->dh_next)
  {
    COUNT nIdx;

    if (!(dhp->dh_attr & ATTR_CHAR))
      continue;

    for (nIdx = 0; nIdx < FNAME_SIZE; ++nIdx)
    {
      if (dhp->dh_name[nIdx] != szName[nIdx])
        break;
    }
    if (nIdx >= FNAME_SIZE)
      return TRUE;
  }

  return FALSE;
}

VOID DosTrimPath(BYTE FAR * lpszPathNamep)
{
  BYTE FAR *lpszLast,
    FAR * lpszNext,
    FAR * lpszRoot = (BYTE FAR *) 0;
  COUNT nChars,
    flDotDot;

  if (*lpszPathNamep == '\\')
    lpszRoot = lpszPathNamep;
  for (lpszNext = lpszPathNamep; *lpszNext; ++lpszNext)
  {
    if (*lpszNext == '/')
      *lpszNext = '\\';
    if (!lpszRoot &&
        *lpszNext == ':' && *(lpszNext + 1) == '\\')
      lpszRoot = lpszNext + 1;
  }

  for (lpszLast = lpszNext = lpszPathNamep, nChars = 0;
       *lpszNext != '\0' && nChars < NAMEMAX;)
  {
    flDotDot = FALSE;

    if (*lpszNext == '\\')
    {
      if (*(lpszNext + 1) == '\\')
        fstrncpy(lpszNext, lpszNext + 1, NAMEMAX);
      else if (*(lpszNext + 1) == '.')
      {
        if (*(lpszNext + 2) == '.'
            && !(*(lpszNext + 3)))
        {
          if (lpszLast == lpszRoot)
            *(lpszLast + 1) = '\0';
          else
            *lpszLast = '\0';
          return;
        }

        if (*(lpszNext + 2) == '.'
            && *(lpszNext + 3) == '\\')
        {
          fstrncpy(lpszLast, lpszNext + 3, NAMEMAX);
          lpszNext = lpszLast;
          if (lpszLast <= lpszPathNamep)
            continue;
          do
          {
            --lpszLast;
          }
          while (lpszLast != lpszPathNamep
                 && *lpszLast != '\\');
          flDotDot = TRUE;
        }
        else if (*(lpszNext + 2) == '\\')
        {
          fstrncpy(lpszNext, lpszNext + 2, NAMEMAX);
        }
        else if (*(lpszNext + 2) == NULL)
        {
          return;
        }
        else
        {
          lpszLast = lpszNext++;
        }
      }
      else
      {
        lpszLast = lpszNext++;
        continue;
      }

      if (!flDotDot)
        lpszLast = lpszNext;
    }
    else
      ++lpszNext;
  }
}
