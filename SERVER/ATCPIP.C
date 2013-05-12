
// ========================================================================
// Arachne TCP/IP init
// (c)1997-1999 Michael Polak, Arachne Labs, xChaos software
// ========================================================================

#include "arachne.h"
#include "internet.h"

void errppp(void)
{
 tcpip=0;
 if(arachne.scriptline)
 {
  sprintf(GLOBAL.location,"file:%s%serr_asf.ah",sharepath,GUIPATH);
  arachne.scriptline=0; //disable scripting!
 }
 else
 {
  sprintf(GLOBAL.location,"file:%s%serr_ppp.ah",sharepath,GUIPATH);
  pagetime=time(NULL);
 }
}

//this is missing in tcp.h header file:
void _arp_add_gateway( char *data , longword ip );

void ArachneTCPIP(void)
{
 char dialer=0;
 char *value=configvariable(&ARACHNEcfg,"Connection",NULL);

//!!glennmcc: Mar 06 2006
char *dns="\0", *dns1="\0", *dns2="\0";
//!!glennmcc: end

 if(!value)
 {
  tcpip=0;
  return;
 }

 if(value[0]=='@')
  dialer=1;

 if(!strncmpi(value,"READY",5))
  tcpip=1;
 else
 if(!strncmpi(value,"HTTPSTUB",8))
  httpstub=1;
 else
 if(!tcpip && dialer && !GLOBAL.location[0])
 {
  value=configvariable(&ARACHNEcfg,"DialPage",NULL);
//!!glennmcc: Begin Feb 06, 2005 -- default to ppp_init.htm
//if 'DialPage' is missing from arachne.cfg
 if(!value) value="file:ppp_init.htm";
//!!glennmcc: end
  if(value)
   strcpy(GLOBAL.location,value);
 }

 if(tcpip) //inicializace TCP/IP pokud je -o, -r ...
 {
  outs(MSG_TCPIP);

  value=configvariable(&ARACHNEcfg,"TCPconfig",NULL);
  if(value)
   tcp_config_file( value);

  value=configvariable(&ARACHNEcfg,"IP_Address",NULL);
  if(!value)
  {
   tcpip=0;
   sprintf(GLOBAL.location,"file:%s%serr_noip.ah",sharepath,GUIPATH);
   return;
  }

#ifndef NOETHERPPP
  if(!strncmpi(value,"PPP",3))
   ipmode=MODE_PPP;
  else
#endif
  if(!strncmpi(value,"BOOTP",5))
   ipmode=MODE_BOOTP;
  else
  if(!strncmpi(value,"WATTCP",5))
   ipmode=MODE_WATTCP;

//  _bootpon = 0;
  switch(ipmode)
  {
   case MODE_NORMAL:

    if(value[0]=='%' || value[0]=='$' )
    {
     char *ptr;
     //enviroment
     makestr(myIPstr,&value[1],19);
     //vymazani zaverecneho '%'
     ptr=strchr(&myIPstr[1],'%');
     if(ptr)
      *ptr='\0';
     makestr(myIPstr,getenv(myIPstr),19);
    }
    else
     makestr(myIPstr,value,19);

    my_ip_addr = resolve( myIPstr );
    break;

   case MODE_BOOTP:
    //_bootpon = 1;
    tcp_config_file( NULL );
    //!!JdS: 2003/12/7 {
    //  If a dial-up connection, initialize 'ppplogtime' via PPPlog().
    //  This allows the "Time Online" status display to function.
    //  Note that PPPlog() has been rewritten to avoid side-effects.
    //!!JdS: 2003/12/7 }
    if(dialer)
     PPPlog();  //failure is tolerable here
    break;

#ifndef NOETHERPPP
   case MODE_PPP:
    if(!PPPlog())
    {
     errppp();
     return;
    }
#endif
  }

  if(sock_init_noexit())
  {
   if(dialer)
    errppp();
   else
   {
    tcpip=0;
    sprintf(GLOBAL.location,"file:%s%serr_pkt.ah",sharepath,GUIPATH);
   }
   return;
  }
  else
   tcpip=1;

/*
  websocket=malloc(sizeof(tcp_Socket));
  if(websocket)
  {
   tcp_listen(websocket, 80, 0, 0, webserver , 0);
  }
*/


  if(ipmode==MODE_PPP)
   loadrefresh=500;

  if(ipmode!=MODE_WATTCP && ipmode!=MODE_BOOTP)
  {
   value=configvariable(&ARACHNEcfg,"Gateway",NULL);
   if(value) _arp_add_gateway( value , 0L );
   value=configvariable(&ARACHNEcfg,"AltGateway",NULL);
   if(value) _arp_add_gateway( value , 0L );
   value=configvariable(&ARACHNEcfg,"Netmask",NULL);
   if(value) sin_mask = resolve( value );
  }

//!!glennmcc: Mar 06 2006 -- modified entire section to use 'dns1' and 'dns2'
//instead of the original 'value' in all places
  dns1=configvariable(&ARACHNEcfg,"NameServer",NULL);
//!!glennmcc: Dec 12, 2005 -- add NameServer %DNS1% capability
  if(dns1 && *dns1=='%')
    {
     char *ptr=strchr(&dns1[1],'%');
     if(ptr) *ptr='\0';
     dns=getenv(&dns1[1]);
    }
  else strcpy(dns,dns1);
//!!glennmcc: end
  if(dns) _add_server( &_last_nameserver, MAX_NAMESERVERS, def_nameservers, resolve(dns));
  dns2=configvariable(&ARACHNEcfg,"AltNameServer",NULL);
//!!glennmcc: Dec 12, 2005 -- add AltNameServer %DNS2% capability
  if(dns2 && *dns2=='%')
    {
     char *ptr=strchr(&dns2[1],'%');
     if(ptr) *ptr='\0';
     dns=getenv(&dns2[1]);
    }
  else strcpy(dns,dns2);
//!!glennmcc: end
  if(dns) _add_server( &_last_nameserver, MAX_NAMESERVERS, def_nameservers, resolve(dns));
 }//endif tcp/ip
}

