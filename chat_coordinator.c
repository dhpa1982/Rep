#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdarg.h>

#include<sys/errno.h>

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>

#define qlen 15

int errexit(const char *format, ...);
void chat(int fd);

struct sess_list
{
    int portnum;
    char message[8];
    struct sess_list* next;
};



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
  char *portnum = "11112";
  int s_udp, n;
  struct sockaddr_in servaddr, cliaddr;
  char message[20];
  char rec_cmd[10];
  char rec_str[9];
  int cliaddr_len;
  int i,j, flag = 0, first = 1;;
  struct sess_list* head = NULL;
  struct sess_list* cur = NULL;
  struct sess_list* tmp = NULL;
  char *choice1 = "Start";
  char *choice2 = "Join";
  char *choice3 = "Terminate";
  pid_t pid;
  int msock;
  int tcp_portnum = 15000;
  int cnt = 1;
  switch(argc)
  {
    case 1:
      break;
    case 2:
      portnum = argv[1];
      break;
    default:
      fprintf(stderr, "usage: input server [port]]\n");
      exit(1);
  }
  memset(&servaddr, 0, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;  //host system IP

  /* Map port number (char string) to port number (int)*/
  if ((servaddr.sin_port=htons((unsigned short)atoi(portnum))) == 0)
    errexit("can't get \"%s\" port number\n", portnum);

  s_udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s_udp < 0)
    errexit("can't create socket: %s\n", strerror(errno));

  bind(s_udp, (struct sockaddr*)&servaddr, sizeof(servaddr));

  while (1)
  {
    cliaddr_len = sizeof(cliaddr);
    for (i=0;i<20;i++)
      message[i] = '\0';
    n = recvfrom(s_udp, message, sizeof(message), 0, (struct sockaddr*)&cliaddr, &cliaddr_len);
    printf("bytes received = %d\n", n);
    message[n] = '\0';
    printf("message received = %s\n", message);
    i = 0;
    j = 0;
    while (message[i] != ' ')
    {
      rec_cmd[i] = message[i];
      i++;
    }
    rec_cmd[i++] = '\0';
    //printf("Cmd Received = %s\n", rec_cmd);
    while(message[i] != '\0')
      rec_str[j++] = message[i++];
    rec_str[j] = '\0';
    //printf("S_name Received = %s\n", rec_str);
    /*cur->next = NULL;
    memset(cur->message, 0, 8);
    cur->message[8] = '\0';*/

    /* Start command from client */
    if(strcmp(rec_cmd, choice1) == 0)
    {
        //printf("comparision ok\n");
        //printf("%s\n", cur->message);
        tmp = head;
        i = 0;
        flag = 0;
        while(tmp != NULL)
        {
            //printf("%d\n", i++);
            printf("%s\n", tmp->message);
            if (strcmp(rec_str, tmp->message) == 0)
            {
              flag = 1;   // s_name already exists
              break;
            }
            tmp = tmp->next;
        }
        if (flag)
        {
          flag = 0;
          printf("Session already running\n");
          message[0] = '-';
          message[1] = '1';
          message[2] = '\0';
          n = sendto(s_udp, message, sizeof(message), 0, (struct sockaddr*)&cliaddr, cliaddr_len);
          printf("bytes sent = %d\n", n);
        }
        else
        {
          cur = malloc(sizeof(struct sess_list));
          printf("Creating tcp socket...\n");

          tcp_portnum++;

          /* Allocate a scoket */
          msock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
          if (msock < 0)
            errexit("can't create socket: %s\n", strerror(errno));

           /* Map port number (char string) to port number (int)*/
          if ((servaddr.sin_port=htons((unsigned short)tcp_portnum)) == 0)
            errexit("can't get \"%s\" port number\n", portnum);


          /* Bind the socket */
          if (bind(msock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
          {
            fprintf(stderr, "can't bind to %s port: %s; Trying other port\n",
                portnum, strerror(errno));
            servaddr.sin_port=htons(0); /* request a port number to be allocated
                                   by bind */
            if (bind(msock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                errexit("can't bind: %s\n", strerror(errno));
            else
            {
                int socklen = sizeof(servaddr);

                if (getsockname(msock, (struct sockaddr *)&servaddr, &socklen) < 0)
                        errexit("getsockname: %s\n", strerror(errno));
            }
          }
          printf("Port number is %d\n", ntohs(servaddr.sin_port));

          /* write the portnum to sess_list */
          //cur = malloc(sizeof(struct sess_list));
          cur->portnum = (int)ntohs(servaddr.sin_port);
          strncpy(cur->message, rec_str, strlen(rec_str));
          cur -> next = head;
          head = cur;          //printf("%s",cur->message)
          printf("listening...\n\n");

          /* listen on the port */
          if (listen(msock, qlen) < 0)
            errexit("can't listen on %s port: %s\n", portnum, strerror(errno));

          printf("msock in coordin = %d\n",msock);

          /* fork session server*/
          if ((pid = fork()) == -1)
          {
            errexit("Process creation failed\n", strerror(errno));
            n = sendto(s_udp, message, sizeof(message), 0, (struct sockaddr*)&cliaddr, cliaddr_len);
            printf("bytes sent = %d\n", n);
          }


          /* child process */
          if (pid == 0)
          {
              printf("[%s]:session server created\n", rec_str);
              execl("chat_server", (char *)itoa(msock, 10),  NULL);
              exit(1);
              /*fd_set rfds;
              fd_set afds;
              int fd, nfds;
              //int n;
              //int msock = atoi(argv[1]);
              char mess[20] = {0};

              nfds = getdtablesize();
              FD_ZERO(&afds);
              FD_SET(msock, &afds);
              while (1)
              {
                printf("Yes\n");
                memcpy(&rfds, &afds, sizeof(rfds));

                if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0,
                                                          (struct timeval *)0) < 0)
                    errexit("select: %s\n", strerror(errno));
                if (FD_ISSET(msock, &rfds))
                {
                  int ssock;
                  ssock = accept(msock, 0, 0);
                  if (ssock < 0)
                    errexit("accept: %s\n", strerror(errno));
                  FD_SET(ssock, &afds);
                  memcpy(&rfds, &afds, sizeof(rfds));
                  printf("Accepted\n");
                }

                for (fd=0; fd<nfds; ++fd)
                {
                  if (fd != msock && FD_ISSET(fd, &rfds))
                    {
                      printf("fd = %d\n", fd);
                      //chat(fd);
                    }
                }
              }*/

          }
          /* Parent process */
          strcpy(message, itoa((int)(ntohs(servaddr.sin_port)),10));
          n = sendto(s_udp, message, sizeof(message), 0, (struct sockaddr*)&cliaddr, cliaddr_len);
          printf("bytes sent = %d\n", n);
        }
    }
    else if(strcmp(rec_cmd, choice2) == 0)
    {
        tmp = head;
        i = 0;
        while(tmp != NULL)
        {
            //printf("%d\n", i++);
            printf("%s\n", tmp->message);
            if (strcmp(rec_str, tmp->message) == 0)
            {
                strcpy(message, itoa(tmp->portnum, 10));
                n = sendto(s_udp, message, sizeof(message), 0, (struct sockaddr*)&cliaddr, cliaddr_len);
                printf("bytes sent = %d\n", n);

                break;
             }
             tmp = tmp->next;
        }
        if (tmp == NULL)
        {
            message[0] = '-';
            message[1] = '1';
            message[2] = '\0';
            n = sendto(s_udp, message, sizeof(message), 0, (struct sockaddr*)&cliaddr, cliaddr_len);
            printf("bytes sent = %d\n", n);
        }
    }
    else if(strcmp(rec_cmd, choice3) == 0)
    {
    }
  }
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

/*while(c < cli_per_ser)
              {
                s_tcp[c] = accept(msock, 0, 0);
                if (s_tcp[c] < 0)
                  errexit("accept: %s\n", strerror(errno));
                read(s_tcp[c], message, sizeof(message));
                printf("[%s]:%s\n", rec_str,  message);
              }*/

/*FD_SET(ssock, &afds);
                  memcpy(&rfds, &afds, sizeof(rfds));
                  if(FD_ISSET(ssock,&rfds))
                  {
                    int k = read(ssock,message,sizeof(message));
                    printf("received message : %s  length : %d\n",message,k);
                  }*/


