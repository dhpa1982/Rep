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

#define cli_per_ser 15

typedef struct list
{
      char message[80];
      char readby[cli_per_ser+1];
      struct list* next;

}mtable;

    mtable* write = NULL;
    mtable* read = NULL;
    mtable* head = NULL;

int
errexit(const char *format, ...)
{
        va_list args;

        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        exit(1);
}
char* itoa(int val, int base)
{
  static char buf[32] = {0};
  int i = 30;
  for(; val && i ; --i, val /= base)
    buf[i] = "0123456789abcdef"[val % base];
  return &buf[i+1];
}


void chat(int fd);

main(int argc, char *argv[])
{
              fd_set rfds;
              fd_set afds;
              int fd, nfds;
              int msock;

              msock = atoi(argv[0]);
              printf("msock in session_ser = %d\n",msock);
              //char mess[20];


              nfds = getdtablesize();
              FD_ZERO(&afds);
              FD_SET(msock, &afds);
              while (1)
              {
                memcpy(&rfds, &afds, sizeof(rfds));
                printf("waiting in select()\n");

                if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0,
                                                          (struct timeval *)0) < 0)
                    errexit("select: %s\n", strerror(errno));
                printf("Done waiting\n");
                if (FD_ISSET(msock, &rfds))
                {
                  int ssock;
                  ssock = accept(msock, 0, 0);
                  if (ssock < 0)
                    errexit("accept: %s\n", strerror(errno));
                  FD_SET(ssock, &afds);
                  //memcpy(&rfds, &afds, sizeof(rfds));
                  printf("Accepted\n");
                  printf("ssock = %d\n",ssock);
                }

                for (fd=0; fd<nfds; ++fd)
                {
                  if (fd != msock && FD_ISSET(fd, &rfds))
                    {
                      printf("fd = %d\n", fd);
                      chat(fd);
                    }
                }
              }
}


void chat(int fd)
{
    int n;
    char mess[100];
    char message[5];
    char *cmd;
    char *mess_len;
    char *text;
    char *choice1 = "Submit";
    char *choice2 = "GetNext";
    char *choice3 = "GetAll";
    char *choice4 = "Leave";

    printf("waiting to receive in chat()\n");
    n = recv(fd, mess, sizeof(mess), 0);
    printf("message = %s : length = %d\n",mess,n);
    cmd = (char *)malloc(7);
    cmd = strtok(mess, " ");
    strcat(cmd,"\0");
    printf("cmd = %s\n", cmd);

    if(strcmp(cmd, choice1) == 0) //Submit
    {
      mess_len = (char *)malloc(3);
      memset(mess_len,'\0',3);
      mess_len = strtok(NULL, " ");
      printf("mess_len = %s\n", mess_len);
      int k = atoi(mess_len);
      printf("k:%d\n",k);
      text = (char *)malloc(k+1);
      text = strtok(NULL, "'\0'");
      strcat(text,"\0");
      printf("text = %s: len = %d\n", text, (int)strlen(text));
      if (head == NULL)
      {
          head = malloc(sizeof(mtable));
          bzero(head->readby, (int)sizeof(head->readby));
          write = head;
      }
      strcpy(write->message, text);
      write->message[strlen(write->message)] = '\0';
      printf("%s\n", write->message);
      if(write->next == NULL)
      {
          printf("malloc\n");
          write->next = malloc(sizeof(mtable));
          bzero(write->readby, (int)sizeof(write->readby));
      }
      write = write->next;
    }
    else if (strcmp(cmd, choice2) == 0) //GetNext
    {
      read = head;
      while(read->next != NULL)
      {
          if (read->readby[fd] == 0)
          {
            memset(mess, '\0', sizeof(mess));
            strcpy(mess, read->message);
            send(fd, mess, sizeof(mess), 0);
            read->readby[fd] = 1;
            break;
          }
            read = read -> next;
      }
      if(read->next == NULL)
      {
        message[0] = '-';
        message[1] = '1';
        message[2] = '\0';
        n = send(fd, message, sizeof(message), 0);
      }
    }
    else if (strcmp(cmd, choice3) == 0) //GetAll
    {
      read = head;
      while(read->next != NULL)
      {
        if (read->readby[fd] == 1)
          read = read -> next;
        else
          break;
      }
      if (read -> next == NULL)
      {
        message[0] = '-';
        message[1] = '1';
        message[2] = '\0';
        n = send(fd, message, sizeof(message), 0);
      }
      else
      {
        mtable* tmp = NULL;
        tmp = read;
        int cnt = 0;
        while(read->next != NULL)
        {
          read = read -> next;
          cnt++;
        }
        printf("cnt = %d\n", cnt);
        char array[3];
        memset(array,'\0',3);
        sprintf(array,"%d",cnt);
        //strcpy(message, (char *)itoa(cnt, 10));
       // strcat(message, "\0");
        //printf("message : %s\n",message);
        send(fd, array, 3, 0);
        read = tmp;
        while(read->next != NULL)
        {
          memset(message, '\0', sizeof(message));
          //printf("read->message:%s\n",read->message);
          cnt = (int)(strlen(read->message));
          //printf("cnt = %d\n", cnt);
          memset(array,'\0',3);
          sprintf(array,"%d",cnt);
          //strcpy(message, itoa(cnt, 10));
         // printf("array = %s\n", array);
          n = send(fd, array, 3, 0);  //message[5]
          //printf("ret = %d\n", n);

          n = send(fd, read->message, strlen(read->message), 0); //[80]
          //printf("ret_mes = %d\n", n);
          read->readby[fd] = 1;
          read = read->next;
        }
      }
    }
    else if(strcmp(cmd, choice4) == 0)
    {

    }
}

