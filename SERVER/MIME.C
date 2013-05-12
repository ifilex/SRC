// ========================================================================
// Management of MIME types according to MIME.CFG
// (c)1997-2000 Michael Polak, Arachne Labs (xChaos software)
// ========================================================================

// when compiling 16 bit, should be always linked STATICALY

#include "arachne.h"

int search_mime_cfg(char *rawmime, char *ext, char *cmd)
{
 char *extptr=NULL,line[IE_MAXLEN],*outext=NULL,*pom;
 int viewer=0;
 char mime[STRINGSIZE],*ptr;

 strcpy(ext,"TMP");
 if(!rawmime || !*rawmime)
  return 0; 
  
  //rawmime is lowecase - 1) strlwr() when reading http, 2) strlwr() when searching for local file
  makestr(mime,rawmime,STRINGSIZE-1);
  ptr=strchr(mime,' ');
  if(ptr)
   *ptr='\0';
  ptr=strchr(mime,';');
  if(ptr)
   *ptr='\0';

  if(!strncmpi(mime,"file/",5) && !strchr(mime,'*'))
  {
   //speed up built in etensions
   extptr=strrchr(mime,'.'); //e.g. mime=file/../blabla.htm
   if(extptr && strstr("HTM TXT GIF BMP IKN htm txt gif bmp ikn",&extptr[1]) )
    goto ret;
  }

  MIMEcfg.y=0;
  while(MIMEcfg.y<MIMEcfg.lines)
  {
   strcpy(line,ie_getline(&MIMEcfg,MIMEcfg.y));

   outext=strchr(line,' ');
   if(outext)
   {
    *outext='\0';
    do
    {
     outext++;
    }
    while(*outext==' ');
   }
   if(!strcmpi(mime,line))
   {
    extptr=strchr(outext,'>');
    if(extptr && strchr(extptr,'|'))
     *extptr='\0';
    else
    {
     extptr=strchr(outext,'|');
     if(extptr)
     {
      *extptr='\0';
      viewer=1;
     }
     else
      extptr=outext; //v nouzi za prikaz povazuji priponu > will be err....
             // tr.: in case of need/emergency I consider extension 
             //      as a command > will be err....
    }
    pom=extptr;
    extptr=outext; // I point at extension
    outext=&pom[1]; // I skip | before cmd
    goto mamji; // e.g. mime=text/html
                // tr.: ('mamji' means: I have got her, i.e. the extension)
   }

   if((!strncmpi(line,"file",4)||
       !strncmpi(line,"ftp",3)/*||
       !strncmpi(line,"fastfile",8)*/) &&
       !strncmpi(line,mime,3)) //pseudomime
   {
    extptr=strchr(line,'/');
    if(extptr)
    {
     extptr++;
     strlwr(extptr);
     strlwr(mime);
     if(strstr(mime,extptr)) // e.g. line=file/.jpg,extptr=.jpg,mime=file/xx.jpg
     {
      if(*outext=='|')
      {
       viewer=1;
       outext++; // skip | before |cmd,
      }
      else
      if( *outext=='>')
       outext++; // skip > before >EXT|cmd..
      else
      if(!cmd)
      {
       extptr=outext;
       goto mamji;
      }
      pom=strrchr(extptr,'.');
      if(pom)
       extptr=pom+1;
      goto mamji;
     }
    }
   }//end if pseudomime

   MIMEcfg.y++;
  }


 if(cmd) // zadny konverzni prikaz neni k dispozici, extenzi nechci...!
   // tr.: there is no command available for conversion,
   //      I do not want this extension
  return 0;

 extptr=strrchr(rawmime,'.'); // e.g. mime=file/../blabla.htm
 if(extptr)
 {
  ret:
  makestr(ext,&extptr[1],3);
  strupr(ext);
 }
#ifdef POSIX
 //in Unix-like systems, text files are usually without extension:
 else
  strcpy(ext,"TXT");
#endif
 return 0;


 mamji:
 makestr(ext,extptr,3);

 if(cmd)
 {
  if(!viewer)
  {
   extptr=strchr(outext,'|');
   if(extptr)
   {
    *extptr='\0';
    makestr(ext,outext,3);
    outext=&extptr[1];
   }
   else
    return 0; //nothing to do
  }
  strcpy(cmd,outext); //strcpy() is safe, because outtext is null-terminated
  return 1+viewer;
 }
 return 0;

}

