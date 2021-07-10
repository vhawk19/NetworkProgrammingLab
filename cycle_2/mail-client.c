/*
SMTP.c: TCP/IP client side, use socket functions.

Compiler: BC++ 3.1, 
          Turbo C++ 1.01(3.01) (free from http://cc.codegear.com/free/cpp)
          
Compile mode: large

Project: Server.c
         ..\..\lib\7186el.lib
         ..\..\lib\tcp_dm32.lib
    
[Nov 03, 2008] by Liam
[Aug,03,2011] by Nicholas
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "..\..\lib\7186e.h"
#include "..\..\lib\Tcpip32.h"

#define BUFSIZE     1024        /* read/write buffer size */
#define SMTP        25
#define SOCKETS     32          // define the maximum number of sockets
#define QLEN    (SOCKETS-1)     // maximum connection queue length

int Quit=0, Quit1=0;
fd_set rfds;                    // set of socket file descriptors for reading

struct sockaddr_in sin;

char *ipaddr="210.241.239.169";  // change for your SMTP server address
struct ip host_ip={{192, 168, 255, 2}, {255, 255, 0, 0}};
int LineMode=1;
char InDataBuf[1460], DataBuf[200];
int DataIdx=0, length, quitmode=0, SmtpMode=0;
char HELO[]="HELLO ICPDAS\r\n";
char CC[50], BCC[50];
char content[1024];
char MAIL[64];
char RCPT[64];


void key_string(char *sinput, char *skeyin)
{
    char data;
    int i=0;
    
    Print("<Current:%s>", sinput);
    
    while((data=Getch())!='\r')
    {
        Putch(data);
        if(data=='\b')
        {
            i--;
            if(i<0)
                i=0;
        }
        else
            skeyin[i++]=data;
    }
    
    if(i>0)
        skeyin[i]=0;
    else
        strcpy(skeyin, sinput);
}

void main(int argc, char *argv[])
{   
    unsigned long t1;
    struct sockaddr_in ssin; /* client Internet endpoing address */
    int sin_len;         /* length of sockaddr_in */
    int client;
    int ss;          /* slave socket got from accept */
    int type, protocol, err=0;
    int rc;          /* return code of selectsocket */
    int i;
    struct timeval   seltime={6,0};
    unsigned ServicePort=SMTP;
    char kin;
    
    InitLib();
    
	sprintf(MAIL,"%s", "liam_lin@icpdas.com");
	sprintf(RCPT,"%s", "liam_lin@icpdas.com");
	sprintf(content,"%s", "hello");
	
  
    // In general it will show 10ms
    while(!Quit)
    {
        Print("\r\n");
        Print("\r\n");
        Print("1.Change the SMTP server.(default:168.95.4.211)\r\n");
        Print("2.Input the 'Mail From' address.(xxx@xxx.xxx.xxx)\r\n");
        Print("3.Input the Mail 'RCPT To' address.(xxx@xxx.xxx.xxx)\r\n");
        Print("4.Input Mail CC address(xxx@xxx.xxx.xxx\r\n");
        Print("5.Input Mail BCC address (xxx@xxx.xxx.xxx)\r\n");
		Print("6.Input the characters as Mail's content:\r\n");
        Print("7.Connect to the SMTP server and send a mail.\r\n");
        Print("Q to exit program.\r\n");
        Print("Please Input your select? ");
        
        kin=Getch();
        Print("%c\n\r", kin);
        
        i=0;
        switch(kin)
        {
            case 'Q':
            case 'q':
                Quit=1;
                break;
            
            case '1':
                key_string(ipaddr,InDataBuf);
                strcpy(ipaddr,InDataBuf);
				Print("\n%s",ipaddr);
                break;
            
            case '2':
                key_string(MAIL,InDataBuf);
                strcpy(MAIL,InDataBuf);
				Print("\n%s",MAIL);
                break;
            
            case '3':
                key_string(RCPT,InDataBuf);
                strcpy(RCPT,InDataBuf);
				Print("\n%s",RCPT);
                break;
            
            case '4':
                key_string(CC,InDataBuf);
                strcpy(CC,InDataBuf);
                Print("\n%s",CC);             
                break;                                              
            
            case '5':
                key_string(BCC,InDataBuf);
                strcpy(BCC,InDataBuf);
                Print("\n%s",BCC);
                break;

			case '6':
				key_string(content,InDataBuf);
				strcpy(content,InDataBuf);
				Print("\n%s",content);
				break;
           
            case '7':
                // mandatory Network Environment 
				err=NetStart();	// initiate host environment
                if(err<0)
                {
                    Print( "initiate host environment failed! error %d\n\r", err );
					Nterm();
                    return;
                }

                err=client=socket(PF_INET, SOCK_STREAM, 0);     // TCP/IP
                if(client<0)
                {
                    Print( "cannot create socket! error %d\n\r", err);
                    Nterm();
                    return;
                }

                // addressing for master socket
                memset(&sin, 0, sizeof(sin));     // bzero is a unix system call
                sin.sin_family=AF_INET;
                sin.sin_addr.s_addr=inet_addr(ipaddr);
                sin.sin_port=htons( ServicePort );

                FD_ZERO(&rfds);

                t1=GetTimeTicks();

                #ifdef _OLD_METHOD_
                    // on success return 0
                    if(connect(client, (struct sockaddr *)&sin, sizeof(sin)))
                    { 
                        extern int errno;
                        Print("connect() failed: %d,time=%lu ms\n", errno,*TimeTicks-t1);
    
                        // when the server does not exist, it will show about 240000ms , that is 4 minutes.
                        // the errno=-12.(ETIMEDOUT)
    
                        return;
                    }
                #else
                    SOCKET_NOBLOCK(client);
                    
                     // on success return 0
                    if(connect(client, (struct sockaddr *)&sin, sizeof(sin)))
                    {
                        extern int errno;
                        unsigned cnt=0;
                        
                        if(errno==EINPROGRESS)
                        {
                            #define ConnectTimeout  4000
                            
                            while(GetTimeTicks()-t1<ConnectTimeout)
                            {
                                YIELD();
                                Print("\r[%u]",++cnt);
                                if(SOCKET_ISOPEN(client))
                                    goto ConnectSuccess;
                            }
                            
                            Print("connect() failed. time=%lu ms,state=%d\n", GetTimeTicks()-t1, 
                                                                              connblo[client].state);
                            
                            return;
                        }
                        else
                        {
                            Print("connect() failed: %d,time=%lu ms\n", errno,*TimeTicks-t1);
                            return;
                        }
                    }

                #endif

ConnectSuccess:
                Print("connect() success, time=%lu ms\n", GetTimeTicks()-t1);
                
                // In general it will show 10ms
                while(!Quit1)
                {
                    FD_SET(client, &rfds);
                    
                    if(selectsocket( SOCKETS, &rfds, NULL, NULL, &seltime)>0)
                    {
                        length=readsocket(client, InDataBuf ,sizeof(InDataBuf));
                        if(length>0)
                        {
                            for(i=0; i<length; i++)
                                Putch(InDataBuf[i]);
                            
                            switch(SmtpMode)
                            {
                                case 0:
                                    Print("Send(%d bytes):%s", sizeof(HELO)-1, HELO);
                                    err=writesocket( client, HELO, sizeof(HELO)-1);
                                    SmtpMode++;
                                    break;
                                
                                case 1:
                                    sprintf(DataBuf,"MAIL FROM:<%s>\r\n",MAIL);
                                    err=writesocket( client,DataBuf,strlen(DataBuf));
                                    SmtpMode++;
                                    break;
                    
                                case 2:
                                    if((strlen(CC)>0)&&(strlen(BCC)>0))
                                        sprintf(DataBuf,"RCPT TO:<%s>;<%s>;<%s>\r\n", RCPT, CC, BCC);
                                    else if(strlen(CC)>0)
                                        sprintf(DataBuf,"RCPT TO:<%s>;<%s>\r\n", RCPT, CC);
                                    else if(strlen(BCC)>0)
                                        sprintf(DataBuf,"RCPT TO:<%s>;<%s>\r\n", RCPT, BCC);
                                    else
                                        sprintf(DataBuf, "RCPT TO:<%s>\r\n", RCPT);
                                        err=writesocket(client, DataBuf, strlen(DataBuf));
                                        SmtpMode++;
                                    break;
                                
                                case 3:
                                    Print("Send:DATA\r\n");
                                    err = writesocket( client, "DATA\r\n", 6);
                                    SmtpMode++;
                                    break;
                                
                                case 4:
                                    sprintf(DataBuf, "From:<%s>\r\nTo:<%s>\r\nCc:<%s>\r\nSubject:This is a SMTP demo\r\n%s\r\n.\r\n", MAIL, RCPT, CC,content);
                                    
									Print(DataBuf);
                                    
                                    err=writesocket(client, DataBuf, strlen(DataBuf));
                                    SmtpMode++;
                                    break;
                                
                                case 5:
                                    Print("Send:QUIT\r\n");
                                    err=writesocket(client, "QUIT\r\n", 6);
                                    SmtpMode++;
									//Quit1=1;
                                    break;
                            }
                        }
                        else if(length==0)
                        {
                            // socket is disconnected
                            Print("Disconnected by remote side.]");
                            Quit1=1;
                        }
                        else
                        {
                            // error
                            Print("[Error:Socket read error!]");
                            Quit1=1;
                        }
                    }
                }
                
                closesocket( client );
                Nterm();
                
                SmtpMode=0;                
                Quit1=0;
                
                break;
        }
    }
}
