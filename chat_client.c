#include<string.h>
#include<stdlib.h>
#include<stdarg.h>
#include<unistd.h>
#include<stdio.h>

#include<sys/errno.h>

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>

#ifndef INADDR_NONE
#define INADDR_NONE     0xffffffff
#endif

#define namelen 9

int errexit(const char *format, ...);
char* itoa(int val, int base)
{
  static char buf[32] = {0};
  int i = 30;
  for(; val && i ; --i, val /= base)
    buf[i] = "0123456789abcdef"[val % base];
  return &buf[i+1];
}


int main(int argc, char *argv[])
{
  char *host = "localhost";
  char *portnum = "11112";
  int s_udp, s_tcp = 0;
  char sel;
  int loop;
  struct hostent  *phe;   // pointer to host information entry
  struct sockaddr_in servaddr; // an Internet endpoint address
  //char message[20] = "Start football";
  char message[20];
  char text[81];
  char  mess[100];
  char str[namelen+1];
  int ret,i=0;

  switch(argc)
  {
    case 1:
      break;
    case 3:
      host = argv[argc-2];
    case 2:
      portnum = argv[argc-1];
      break;
    default:
      fprintf(stderr, "usage: input server [host [port]]\n");
      exit(1);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;

  /* Map port number (char string) to port number (int)*/
  if ((servaddr.sin_port=htons((unsigned short)atoi(portnum))) == 0)
    errexit("can't get \"%s\" port number\n", portnum);

  /* Map host name to IP address, allowing for dotted decimal */
  if ( phe = gethostbyname(host) )
    memcpy(&servaddr.sin_addr, phe->h_addr, phe->h_length);
  else if ( (servaddr.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
    errexit("can't get \"%s\" host entry\n", host);

  /* Allocate a socket */
  s_udp = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s_udp < 0)
    errexit("can't create socket: %s\n", strerror(errno));

  do
  {
    loop = 0;
    char len[3];
    memset(len,'\0',3);
    printf("\r1.Start\n\r2.Join\n\r3.Submit\n\r4.GetNext\n\r");
    printf("5.GetAll\n\r6.Leave\n\r7.Exit\n\rEntern your selection:");
    scanf("%c", &sel);
    getchar();
    switch(sel)
    {
      case '1'://start
        if (s_tcp>0)
        {
            printf("Closing the current chat session\n");
            close(s_tcp);
        }
        printf("Enter the name for the chat session(max 8 characters):");
        fgets(str, namelen+1, stdin);
        str[strlen(str)-1] = '\0';
        printf("string = %s \n", str);
        strcpy(message, "Start ");
        strcat(message, str);
        printf("message = %s\n", message);
        ret = sendto(s_udp, message, sizeof(message), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
        printf("Sent %d bytes\n", ret);
        ret = recvfrom(s_udp, message, sizeof(message), 0, 0, 0);
        printf("receiving...(-1 or tcp port number)\n");
        //printf("bytes received = %d\n", ret);
        printf("%s\n", message);
        if (strcmp(message, "-1") == 0)
        {
          printf("Session name already exist\n");
          break;
        }
        if ((servaddr.sin_port=htons((unsigned short)atoi(message))) == 0)
          errexit("can't get \"%s\" port number\n", portnum);

        /* Allocate a socket */
        s_tcp = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s_tcp < 0)
                errexit("can't create socket: %s\n", strerror(errno));

        /* Connect the socket */
        if (connect(s_tcp, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                errexit("can't connect to %s.%s: %s\n", host, portnum, strerror(errno));
        //sleep(1);
        //ret = send(s_tcp, message, sizeof(message), 0);
        //printf("bytes sent = %d\n", ret);
        printf("A new chat session %s has been created and you have joined the session\n", str);
        loop = 1;
        break;
      case '2':// Join
        printf("Enter the name of the chat session(max 8 characters):");
        fgets(str, namelen+1, stdin);
        str[strlen(str)-1] = '\0';
        printf("string = %s \n", str);
        strcpy(message, "Join ");
        strcat(message, str);
        printf("message = %s\n", message);
        ret = sendto(s_udp, message, sizeof(message), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
        printf("Sent %d bytes\n", ret);
        ret = recvfrom(s_udp, message, sizeof(message), 0, 0, 0);
        printf("receiving...(-1 or tcp port number)\n");
        //printf("bytes received = %d\n", ret);
        printf("%s\n", message);
        if (strcmp(message, "-1") == 0)
        {
          printf("Session name does not exist\n");
          break;
        }
        if ((servaddr.sin_port=htons((unsigned short)atoi(message))) == 0)
          errexit("can't get \"%s\" port number\n", portnum);

        /* Allocate a socket */
        s_tcp = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s_tcp < 0)
                errexit("can't create socket: %s\n", strerror(errno));

        /* Connect the socket */
        if (connect(s_tcp, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                errexit("can't connect to %s.%s: %s\n", host, portnum, strerror(errno));

        printf("You have joined the session %s\n", str);
        //write(s_tcp, message, sizeof(message));
        loop = 1;

        break;
      case '3':// submit
        memset(mess, '\0', sizeof(mess));
        printf("Enter the message you want to send:");
        fgets(text, 80, stdin);
        int l = strlen(text);
        text[l-1] = '\0';
        strcat(mess, "Submit ");
        //printf("%s\n", mess);
        strcat(mess, itoa(l, 10));
        strcat(mess, " ");
        //printf("%s\n", mess);
        strcat(mess, text);
        printf("%s\n", mess);
        send(s_tcp, mess, sizeof(mess), 0);
        loop = 1;
        break;
      case '4': // GetNext
        memset(message, '\0', sizeof(message));
        strcpy(message, "GetNext ");
         send(s_tcp, message, sizeof(message), 0);
       // printf("return val of send: %d\n",k);
       recv(s_tcp, mess, sizeof(mess), 0);
        //printf("return val of recv: %d\n",k);
        if (strcmp(mess, "-1") == 0)
          printf("No more messages\n");
        printf("\nmessage = %s\n\n", mess);
        loop =1;
        break;
      case '5'://GetAll
        memset(message, '\0', sizeof(message));
        strcpy(message, "GetAll ");
        //memset(message,'\0',sizeof(message));
        strcat(message,"\0");
        int k = send(s_tcp, message, sizeof(message), 0);
        //printf("return val of send: %d\n",k);
        k = recv(s_tcp, len, 3, 0);
        //printf("return val of recv: %d\n",k);

        //send(s_tcp, message, sizeof(message), 0);
        //recv(s_tcp, message, sizeof(message), 0);
        if (strcmp(mess, "-1") == 0)
        {
          printf("No more messages\n");
          break;
        }
        int num_mess =0,i=0;
        num_mess = atoi(len);
        printf("num_mess = %d\n", num_mess);
        while(num_mess--)
        {
          ret = recv(s_tcp, len, sizeof(len), 0); //[20]
          //printf("len = %s: ret = %d\n", len,ret);
          int a = atoi(len);
          //printf("received length of message %d = %d\t",i+1, a);
          memset(mess,'\0',sizeof(mess));
          ret = recv(s_tcp, mess, a, 0); //[100]
          //if (strlen(mess) == a);
          //  printf("message recv properly\n");
          printf("recv mess = %s\n", mess);
          i++;
        }
        loop =1;

        break;
      case '6':
        break;
      case '7':
        loop=0;
        break;
      default:
        loop = 1;
    }
  }while(loop);
  close(s_udp);
  close(s_tcp);
}

int
errexit(const char *format, ...)
{
        va_list args;

        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        exit(1);
}

