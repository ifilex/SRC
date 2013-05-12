/* This is EXE to binary (memory image) file converter. It is a part of FreeDOS */
/* See the GNU licence for details on your rights                             */
/* Written and (c) RaMax (Raevski Maxim)                                      */
/* For problem reports e-mail: ramax@compuserve.com or                        */
/* 101321.1461@compuserve.com. Please use "EXE2BIN" as a subject line         */
/* This program is written using Borland C++. MAKE file is enclosed           */
/* Commented source file and documentation will follow soon                   */

#include <stdio.h>
#include <stdlib.h>
#include <dir.h>
#include <string.h>
#include <fcntl.h>

FILE *exe, *binary;

unsigned int Signature, PartPage, PageCount, ReloCount, HeaderSize, MinMem,
                                 MaxMem, ReloSS, ExeSP, ChkSum, ExeIP, ReloCS, TableOffset, Overlay;

/* Variables above are used to store the information from EXE file header     */
/* Description follows:                                                       */

/* Signature: "MZ" or "ZM"  standard EXE file signature. Not EXE if absent.   */
/* PartPage: Length of partial page at the end of the file (page=512 bytes)   */
/* PageCount: length of file in 512-byte pages, inc of header                 */
/* ReloCount: Number of items in relocation table (must be 0 to be successfully
                                  cobverted to binary file                                        */
/* HeaderSize: Size of the header inc relocation table in paragraphs (16 byte)*/
/* MinMem:  Minimum of memory required to run (ignored during conversion)     */
/* MaxMem:  Maximum memory reqired at start   (ignored during conversion)     */
/* ReloSS:      Segment offset for stack segment. Must be 0 (my guess) for .COM   */
/* ExeSP:   Value for SP register. Must be 0FFFFh.                            */
/*    ***Note: In fact, I am not worried if values in header do not match what
                I expect. I only warn the user that stack might be present. If anyone
                knows what really must be in these fields, drop me a line               */
/* ChkSum: file checksum. I ignore this value. Anyone knows the algorythm?    */
/* ExeIP:  points to entry point of the program. Must be 100h                 */
/* ReloCS: segment offset in file. Must be 0                                  */
/* TableOffset: file offset of the relocation table (often 1Ch)               */
/* Overlay: overlay number (0 if main module). I don't care.                  */



/* The utility returns error codes as following:
        0=OK
        1=File not found
        2=Cannot convert the specified file
        3=Relocatable items present. Cannot proceed.
        4=Stack present. Please be careful.
        5=The file is too big. Cannot proceed.
        6=No parameters specified
        7=Not EXE file. Cannot proceed.
        8=Wrong entry point
        9=Incorrect parameters
*/

unsigned int bytecount;
unsigned long proglen, progoffset;
unsigned char buffer;

int main(int argc, char *argv[])
{
        int MergeResult;
        char Drive[MAXDRIVE], Dir[MAXDIR], FileName[MAXFILE];
        char FullName[MAXPATH]="\0", Ext[MAXEXT]="\0";

        _fmode=O_BINARY;
        printf ("EXE to BINary converter. Copyright (c) RaMax 1995\n\r Part of FreeDOS.\n\r");

        if (argc==1)
        {
         printf("  Usage: EXE2BIN in_file[.exe] out_file[.bin]");
         return 6;
        }


        MergeResult=fnsplit (argv[1], Drive, Dir, FileName, Ext);

        if ((MergeResult & WILDCARDS)!=0)
        {
         printf ( "Wildcards are not supported.\n\r");
         return 9;
        }


        if ((MergeResult & EXTENSION)==0) strcpy (Ext,".EXE");
        else if (strcmp (strupr (Ext),".EXE")!=0)
                         printf ( "File's extension is NOT '.EXE'. Result is not guaranteed.\n\r");

        fnmerge (FullName, Drive, Dir, FileName, Ext);




        exe=fopen(FullName,"r");
        if (exe==NULL)
        {
         printf("Cannot find/open the specified file \n\r");
         return 1;
        }

        bytecount=fread(&Signature, sizeof(Signature), 1, exe);

        if ((Signature !=0x5a4dL) && (Signature !=0x4d5aL))
        {
         fclose(exe);
         printf("File specified is not in EXE format.\n\r");
         return 7;
        }

        bytecount=fread(&PartPage,2,1,exe);
        bytecount=fread(&PageCount,2,1,exe);
        bytecount=fread(&ReloCount,2,1,exe);
        bytecount=fread(&HeaderSize,2,1,exe);
        bytecount=fread(&MinMem,2,1,exe);
        bytecount=fread(&MaxMem,2,1,exe);
        bytecount=fread(&ReloSS,2,1,exe);
        bytecount=fread(&ExeSP,2,1,exe);
        bytecount=fread(&ChkSum,2,1,exe);
        bytecount=fread(&ExeIP,2,1,exe);
        bytecount=fread(&ReloCS,2,1,exe);
        bytecount=fread(&TableOffset,2,1,exe);
        bytecount=fread(&Overlay,2,1,exe);

        if (ReloCount!=0)
        {
         printf ("This file contains relocatable elements. Conversion impossible\n\r");
         fclose (exe);
         return 3;
        }

        if (ExeIP!=0x100)
        {
         printf ("The entry point of this file is not 100h. Conversion impossible\n\r");
         fclose (exe);
         return 8;
        }

        if (TableOffset !=0x1c)
                 printf ("This file might be using enhanced EXE format.\n\r");

        if (ReloSS!=0 || ExeSP!=0)
                 printf ("This file might contain predefined stack.\n\r");

		  proglen=((PageCount*512)-(HeaderSize*16))-PartPage;

		  if (proglen>65535L)
		  {
			printf ( "File is too long. Conversion impossible.\n\r");
			fclose(exe);
			return 5;
		  }

		  printf ("This file's length is %u\n\r",proglen);
		  progoffset = HeaderSize*16+0x100;


		  if (argc>=3)
		  {
			MergeResult=fnsplit (argv[2], Drive, Dir, FileName, Ext);

			if ((MergeResult & WILDCARDS)!=0)
			{
			 printf ( "Wildcards cannot be used here.\n\r");
			 return 9;
			}
		  }


		  if ((MergeResult & EXTENSION)==0 || argc<3)
				strcpy (Ext,".BIN");
		  fnmerge (FullName, Drive, Dir, FileName, Ext);
		  binary=fopen(FullName,"w");
		  if (binary==NULL)
		  {
			printf ( "Cannot open the output file.\n\r");
			fclose (exe);
			return 2;
		  }

		  if (fseek(exe, progoffset, SEEK_SET))
		  {
			printf ( "Some file problems occurred\n\r");
			fclose (exe);
			fclose (binary);
			return 2;
		  }
		  while (1)
		  {
			bytecount= fread (&buffer, 1, 1, exe);
			if (!feof(exe)) bytecount= fwrite (&buffer, 1, 1, binary);
         else break;
		  }
		  fclose (exe);
		  fclose (binary);
		  printf ( "File was successfully converted\n\r");
		  return 0;
}
