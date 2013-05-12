#define INIT

#ifdef IN_INIT_MOD
#  define __FAR_WRAPPER(ret, name, proto) \
        ret FAR name proto; 
#else
#  define __FAR_WRAPPER(ret, name, proto) \
	ret name proto; \
	ret FAR init_call_##name proto;
#endif

VOID FAR init_buffers(void);
ULONG getblkno(struct buffer FAR *);
VOID setblkno(struct buffer FAR *, ULONG);
struct buffer FAR *getblock(ULONG blkno, COUNT dsk);
BOOL getbuf(struct buffer FAR ** pbp, ULONG blkno, COUNT dsk);
VOID setinvld(REG COUNT dsk);
BOOL flush_buffers(REG COUNT dsk);
BOOL flush1(struct buffer FAR * bp);
BOOL flush(void);
BOOL fill(REG struct buffer FAR * bp, ULONG blkno, COUNT dsk);
BOOL dskxfer(COUNT dsk, ULONG blkno, VOID FAR * buf, UWORD numblocks, COUNT mode);

VOID cso(COUNT c);
VOID sto(COUNT c);
VOID mod_sto(REG UCOUNT c);
VOID destr_bs(void);
UCOUNT _sti(void);
VOID con_hold(void);
BOOL con_break(void);
BOOL StdinBusy(void);
VOID KbdFlush(void);
VOID Do_DosIdle_loop(void);
__FAR_WRAPPER(VOID, sti, (keyboard FAR * kp))

sft FAR *get_sft(COUNT);

INIT VOID PreConfig(VOID);
INIT VOID DoConfig(VOID);
INIT VOID PostConfig(VOID);
INIT BYTE *skipwh(BYTE * s);
INIT BYTE *scan(BYTE * s, BYTE * d);
INIT BOOL isnum(BYTE * pszString);
INIT BYTE *GetNumber(REG BYTE * pszString, REG COUNT * pnNum);
INIT COUNT tolower(COUNT c);
INIT COUNT toupper(COUNT c);
INIT VOID mcb_init(mcb FAR * mcbp, UWORD size);
INIT VOID strcat(REG BYTE * d, REG BYTE * s);

BOOL check_break(void);
UCOUNT GenericRead(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err,
  BOOL force_binary);
UCOUNT DosRead(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err);
UCOUNT DosWrite(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err);
COUNT DosSeek(COUNT hndl, LONG new_pos, COUNT mode, ULONG * set_pos);
COUNT DosCreat(BYTE FAR * fname, COUNT attrib);
COUNT CloneHandle(COUNT hndl);
COUNT DosDup(COUNT Handle);
COUNT DosForceDup(COUNT OldHandle, COUNT NewHandle);
COUNT DosOpen(BYTE FAR * fname, COUNT mode);
COUNT DosClose(COUNT hndl);
VOID DosGetFree(COUNT drive, COUNT FAR * spc, COUNT FAR * navc, COUNT FAR * bps, COUNT FAR * nc);
COUNT DosGetCuDir(COUNT drive, BYTE FAR * s);
COUNT DosChangeDir(BYTE FAR * s);
COUNT DosFindFirst(UCOUNT attr, BYTE FAR * name);
COUNT DosFindNext(void);
COUNT DosGetFtime(COUNT hndl, date FAR * dp, time FAR * tp);
COUNT DosSetFtime(COUNT hndl, date FAR * dp, time FAR * tp);
COUNT DosGetFattr(BYTE FAR * name, UWORD FAR * attrp);
COUNT DosSetFattr(BYTE FAR * name, UWORD FAR * attrp);

VOID DosIdle_int(void);

VOID SpacePad(BYTE *, COUNT);
COUNT ParseDosName(BYTE FAR *, COUNT *, BYTE *, BYTE *, BYTE *, BOOL);
COUNT ParseDosPath(BYTE FAR *, COUNT *, BYTE *, BYTE *);
BOOL IsDevice(BYTE * FileName);

COUNT blk_driver(rqptr rp);

VOID dump(void);
VOID panic(BYTE * s);
__FAR_WRAPPER(VOID, fatal, (BYTE * err_msg))
COUNT char_error(request * rq, struct dhdr FAR * lpDevice);
COUNT block_error(request * rq, COUNT nDrive, struct dhdr FAR * lpDevice);

struct f_node FAR *dir_open(BYTE FAR * dirname);
COUNT dir_read(REG struct f_node FAR * fnp);
COUNT dir_write(REG struct f_node FAR * fnp);
VOID dir_close(REG struct f_node FAR * fnp);
COUNT dos_findfirst(UCOUNT attr, BYTE FAR * name);
COUNT dos_findnext(void);

__FAR_WRAPPER(COUNT, dos_open, (BYTE FAR * path, COUNT flag))
BOOL fcmp(BYTE FAR * s1, BYTE FAR * s2, COUNT n);
BOOL fcmp_wild(BYTE FAR * s1, BYTE FAR * s2, COUNT n);
VOID touc(BYTE FAR * s, COUNT n);
__FAR_WRAPPER(COUNT, dos_close, (COUNT fd))
COUNT dos_creat(BYTE FAR * path, COUNT attrib);
COUNT dos_delete(BYTE FAR * path);
COUNT dos_rmdir(BYTE FAR * path);
COUNT dos_rename(BYTE FAR * path1, BYTE FAR * path2);
__FAR_WRAPPER(date, dos_getdate, (void))
__FAR_WRAPPER(time, dos_gettime, (void))
COUNT dos_getftime(COUNT fd, date FAR * dp, time FAR * tp);
COUNT dos_setftime(COUNT fd, date FAR * dp, time FAR * tp);
LONG dos_getcufsize(COUNT fd);
LONG dos_getfsize(COUNT fd);
BOOL dos_setfsize(COUNT fd, LONG size);
COUNT dos_mkdir(BYTE FAR * dir);
BOOL last_link(struct f_node FAR * fnp);
COUNT map_cluster(REG struct f_node FAR * fnp, COUNT mode);
UCOUNT readblock(COUNT fd, VOID FAR * buffer, UCOUNT count, COUNT * err);
UCOUNT writeblock(COUNT fd, VOID FAR * buffer, UCOUNT count, COUNT * err);
__FAR_WRAPPER(COUNT, dos_read, (COUNT fd, VOID FAR * buffer, UCOUNT count))
COUNT dos_write(COUNT fd, VOID FAR * buffer, UCOUNT count);
LONG dos_lseek(COUNT fd, LONG foffset, COUNT origin);
UWORD dos_free(struct dpb *dpbp);
VOID dos_pwd(struct dpb *dpbp, BYTE FAR * s);
VOID trim_path(BYTE FAR * s);
COUNT dos_cd(struct dpb *dpbp, BYTE FAR * s);
struct f_node FAR *get_f_node(void);
VOID release_f_node(struct f_node FAR * fnp);
VOID dos_setdta(BYTE FAR * newdta);
COUNT dos_getfattr(BYTE FAR * name, UWORD FAR * attrp);
COUNT dos_setfattr(BYTE FAR * name, UWORD FAR * attrp);
COUNT media_check(REG struct dpb *dpbp);
struct f_node FAR *xlt_fd(COUNT fd);
COUNT xlt_fnp(struct f_node FAR * fnp);
struct dhdr FAR *select_unit(COUNT drive);

UCOUNT link_fat(struct dpb *dpbp, UCOUNT Cluster1, REG UCOUNT Cluster2);
UCOUNT link_fat16(struct dpb *dpbp, UCOUNT Cluster1, UCOUNT Cluster2);
UCOUNT link_fat12(struct dpb *dpbp, UCOUNT Cluster1, UCOUNT Cluster2);
UWORD next_cluster(struct dpb *dpbp, REG UCOUNT ClusterNum);
UWORD next_cl16(struct dpb *dpbp, REG UCOUNT ClusterNum);
UWORD next_cl12(struct dpb *dpbp, REG UCOUNT ClusterNum);

VOID DosOutputString(BYTE FAR * s);
int DosCharInputEcho(VOID);
int DosCharInput(VOID);
VOID DosDirectConsoleIO(iregs FAR * r);
VOID DosCharOutput(COUNT c);
VOID DosDisplayOutput(COUNT c);
VOID FatGetDrvData(COUNT drive, COUNT FAR * spc, COUNT FAR * bps, COUNT FAR * nc, BYTE FAR ** mdp);
WORD FcbParseFname(int wTestMode, BYTE FAR ** lpFileName, fcb FAR * lpFcb);
BYTE FAR *ParseSkipWh(BYTE FAR * lpFileName);
BOOL TestCmnSeps(BYTE FAR * lpFileName);
BOOL TestFieldSeps(BYTE FAR * lpFileName);
BYTE FAR *GetNameField(BYTE FAR * lpFileName, BYTE FAR * lpDestField, COUNT nFieldSize, BOOL * pbWildCard);
BOOL FcbRead(xfcb FAR * lpXfcb, COUNT * nErrorCode);
BOOL FcbWrite(xfcb FAR * lpXfcb, COUNT * nErrorCode);
BOOL FcbGetFileSize(xfcb FAR * lpXfcb);
BOOL FcbSetRandom(xfcb FAR * lpXfcb);
BOOL FcbCalcRec(xfcb FAR * lpXfcb);
BOOL FcbRandomBlockRead(xfcb FAR * lpXfcb, COUNT nRecords, COUNT * nErrorCode);
BOOL FcbRandomBlockWrite(xfcb FAR * lpXfcb, COUNT nRecords, COUNT * nErrorCode);
BOOL FcbRandomRead(xfcb FAR * lpXfcb, COUNT * nErrorCode);
BOOL FcbRandomWrite(xfcb FAR * lpXfcb, COUNT * nErrorCode);
BOOL FcbCreate(xfcb FAR * lpXfcb);
void FcbNameInit(fcb FAR * lpFcb, BYTE * pszBuffer, COUNT * pCurDrive);
BOOL FcbOpen(xfcb FAR * lpXfcb);
BOOL FcbDelete(xfcb FAR * lpXfcb);
BOOL FcbRename(xfcb FAR * lpXfcb);
void MoveDirInfo(dmatch FAR * lpDmatch, struct dirent FAR * lpDir);
BOOL FcbClose(xfcb FAR * lpXfcb);
BOOL FcbFindFirst(xfcb FAR * lpXfcb);
BOOL FcbFindNext(xfcb FAR * lpXfcb);

UWORD init_oem(void);

VOID INRPT far got_cbreak(void);    
VOID INRPT far int20_handler(iregs UserRegs);
VOID INRPT far int21_handler(iregs UserRegs);
VOID far int21_entry(iregs UserRegs);
VOID int21_service(iregs far * r);
VOID INRPT FAR int22_handler(void);
VOID INRPT FAR int23_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags);
VOID INRPT FAR int24_handler(void);
VOID INRPT FAR low_int25_handler(void);
VOID INRPT FAR low_int26_handler(void);
VOID int25_handler();
VOID int26_handler();
VOID INRPT FAR int27_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags);
VOID INRPT FAR int28_handler(void);
VOID INRPT FAR int2a_handler(void);
VOID INRPT FAR int2f_handler(void);
VOID INRPT FAR empty_handler(void);

COUNT DosDevIOctl(iregs FAR * r, COUNT FAR * err);

INIT VOID main(void);
INIT VOID init_device(struct dhdr FAR * dhp, BYTE FAR * cmdLine);
INIT struct dhdr FAR *link_dhdr(struct dhdr FAR * lp, struct dhdr FAR * dhp, BYTE FAR * cmdLine);

seg far2para(VOID FAR * p);
seg long2para(LONG size);
VOID FAR *add_far(VOID FAR * fp, ULONG off);
VOID FAR *adjust_far(VOID FAR * fp);
__FAR_WRAPPER(COUNT, DosMemAlloc,
  (UWORD size, COUNT mode, seg FAR * para, UWORD FAR * asize))
COUNT DosMemLargest(UWORD FAR * size);
COUNT DosMemFree(UWORD para);
COUNT DosMemChange(UWORD para, UWORD size, UWORD * maxSize);
COUNT DosMemCheck(void);
COUNT FreeProcessMem(UWORD ps);
COUNT DosGetLargestBlock(UWORD FAR * block);
VOID show_chain(void);
VOID mcb_print(mcb FAR * mcbp);
VOID _fmemcpy(BYTE FAR * d, BYTE FAR * s, REG COUNT n);

__FAR_WRAPPER(VOID, scopy, (REG BYTE * s, REG BYTE * d))
VOID fscopy(REG BYTE FAR * s, REG BYTE FAR * d);
VOID fsncopy(BYTE FAR * s, BYTE FAR * d, REG COUNT n);
VOID bcopy(REG BYTE * s, REG BYTE * d, REG COUNT n);
__FAR_WRAPPER(VOID, fbcopy, (REG VOID FAR * s, REG VOID FAR * d, REG COUNT n))

UWORD GetCtryInfo(UBYTE FAR * lpShrtCode, UWORD FAR * lpLongCode, BYTE FAR * lpTable);
UWORD SetCtryInfo(UBYTE FAR * lpShrtCode, UWORD FAR * lpLongCode, BYTE FAR * lpTable, UBYTE * nRetCode);
UWORD internalUpcase(UWORD c);
BOOL GetGlblCodePage(UWORD FAR * ActvCodePage, UWORD FAR * SysCodePage);
BOOL SetGlblCodePage(UWORD FAR * ActvCodePage, UWORD FAR * SysCodePage);
BOOL ExtCtryInfo(UBYTE nOpCode, UWORD CodePageID, UWORD InfoSize, VOID FAR * Information);
char upMChar(UPMAP map, char ch);
VOID upMMem(UPMAP map, char FAR * str, unsigned len);
BYTE yesNo(char ch);
char upChar(char ch);
VOID upString(char FAR * str);
VOID upMem(char FAR * str, unsigned len);
char upFChar(char ch);
VOID upFString(char FAR * str);
VOID upFMem(char FAR * str, unsigned len);
BOOL FAR LoadCountryInfo(char FAR * filename, WORD ctryCode, WORD codePage);

VOID put_console(COUNT c);
__FAR_WRAPPER(WORD, printf, (CONST BYTE * fmt, ...))
WORD sprintf(BYTE * buff, CONST BYTE * fmt,...);

__FAR_WRAPPER(COUNT, strlen, (REG BYTE * s))
COUNT fstrlen(REG BYTE FAR * s);
VOID _fstrcpy(REG BYTE FAR * d, REG BYTE FAR * s);
VOID strncpy(REG BYTE * d, REG BYTE * s, COUNT l);
__FAR_WRAPPER(COUNT, strcmp, (REG BYTE * d, REG BYTE * s))
COUNT fstrcmp(REG BYTE FAR * d, REG BYTE FAR * s);
COUNT fstrncmp(REG BYTE FAR * d, REG BYTE FAR * s, COUNT l);
COUNT strncmp(REG BYTE * d, REG BYTE * s, COUNT l);
VOID fstrncpy(REG BYTE FAR * d, REG BYTE FAR * s, COUNT l);
BYTE * strchr(BYTE * s, BYTE c);

WORD clk_driver(rqptr rp);
COUNT BcdToByte(COUNT x);
COUNT BcdToWord(BYTE * x, UWORD * mon, UWORD * day, UWORD * yr);
COUNT ByteToBcd(COUNT x);
LONG WordToBcd(BYTE * x, UWORD * mon, UWORD * day, UWORD * yr);

WORD con_driver(rqptr rp);
VOID break_handler(void);
VOID INRPT FAR int29_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags);

VOID getdirent(BYTE FAR * vp, struct dirent FAR * dp);
VOID putdirent(struct dirent FAR * dp, BYTE FAR * vp);

VOID DosGetTime(BYTE FAR * hp, BYTE FAR * mp, BYTE FAR * sp, BYTE FAR * hdp);
COUNT DosSetTime(BYTE FAR * hp, BYTE FAR * mp, BYTE FAR * sp, BYTE FAR * hdp);
VOID DosGetDate(BYTE FAR * wdp, BYTE FAR * mp, BYTE FAR * mdp, COUNT FAR * yp);
COUNT DosSetDate(BYTE FAR * mp, BYTE FAR * mdp, COUNT FAR * yp);

COUNT ChildEnv(exec_blk FAR * exp, UWORD * pChildEnvSeg, char far * pathname);
VOID new_psp(psp FAR * p, int psize);
VOID return_user(void);
__FAR_WRAPPER(COUNT, DosExec, (COUNT mode, exec_blk FAR * ep, BYTE FAR * lp))
VOID FAR p_0(void);

VOID init_stacks(VOID FAR * stack_base, COUNT nStacks, WORD stackSize);

int SetJFTSize(UWORD nHandles);
int DosMkTmp(BYTE FAR * pathname, UWORD attr);
int truename(char FAR * src, char FAR * dest);

UWORD get_machine_name(BYTE FAR * netname);
VOID set_machine_name(BYTE FAR * netname, UWORD name_num);


VOID INRPT FAR exec_user(iregs FAR * irp);

#define strcpy(d, s)	scopy(s, d)
