/* General Utilities */
#include <stdio.h>
#include <stdlib.h>

/* Errors and Interupts */
#include <signal.h>
#include <errno.h>

/* socket stuff */
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>

/* Select Stuff */
#include <sys/time.h>
#include <unistd.h>


/* program specific stuff */
#include "smartalloc.h"
//#include "json-server.h"
//#include "parser.h"


#define READ 0
#define WRITE 1
#define BS 256

int *Sfds;
int Count = 0;

typedef struct buf
{
   char buffer[BS];
} buf;

int *ReadLen;
int *WritLen;
buf *ReadBuf;
buf *WritBuf;



void server_shutdown()
{
   free(Sfds);
   printf("Server exiting cleanly\n");
   fflush(stdout);
   exit (0);
}

void sigint_handler(int sig)
{
   if (SIGINT == sig)
      server_shutdown();
}

void set_sigint()
{
   struct sigaction sa;   
   sa.sa_handler = sigint_handler;
   sigfillset(&sa.sa_mask);
   sa.sa_flags = 0;
   if (-1 == sigaction(SIGINT, &sa, NULL))
   {
      fprintf(stderr, "set_sigint: unable to initialize\n");
      fprintf(stderr, "Ernor: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
   }
}

int gen_socket()
{
   int sfd = socket(AF_INET6, SOCK_STREAM, 0);
   if (sfd <= 0)
   {
      fprintf(stderr, "gen_socket: creation failed\n");
      fprintf(stderr, "Ernor: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
   }

   int opt = 1;
   if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
   {
      fprintf(stderr, "gen_socket: settings failed\n");
      fprintf(stderr, "Errno: %s", strerror(errno));
      exit(EXIT_FAILURE);
   }
   return sfd;
}


void gen_addr(int argc, char *argv[], struct sockaddr_in6 *addr)
{
   char map46[7+16+1];

   addr->sin6_family = AF_INET6;

   if (argc != 2)
   {
      addr->sin6_addr = in6addr_any;
   }
   else
   {
      if (!inet_pton(AF_INET6, argv[1], &(addr->sin6_addr)))
      {
         sprintf(map46, "::FFFF:%s", argv[1]);
         if (!inet_pton(AF_INET6, map46, &(addr->sin6_addr)))
         {
            fprintf(stderr, "gen_addr: invalid address %s\n", map46);
            fprintf(stderr, "Errno: %s", strerror(errno));
            exit(EXIT_FAILURE);
         }
      }
   }
   addr->sin6_port = htons(0);
}

void bind_socket(int sfd, struct sockaddr_in6 *addr)
{
   if (bind(sfd, (struct sockaddr *)addr, sizeof(*addr)) < 0)
   {
      fprintf(stderr, "bind_socket: Socket Bind Failure\n");
      fprintf(stderr, "Erno: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
   }


   struct sockaddr_in6 foo;
   unsigned int len = sizeof(struct sockaddr);
   getsockname(sfd, (struct sockaddr *) &foo, &len);
   char straddr[INET6_ADDRSTRLEN];
   inet_ntop(AF_INET6, &foo.sin6_addr, straddr, sizeof(straddr));

   printf ("HTTP server is using TCP port %d\n", ntohs(foo.sin6_port));
   printf ("HTTPS server is using TCP port -1\n");
   fflush(stdout);
}

/* extend the list of sockets to include a new one */
void add_array(int *sfds, int *count, int *cap, int new_sfd)
{
   /* If the vacant spots start to encroach on capacity double it */
   if (*count > *cap / 2)
   {
      *cap *= 2;
      sfds = realloc(sfds, *cap * sizeof (int));

      /* print out the array for debugging purposes */
      for (int i = 0; i < *count; i++)
      {
         fprintf(stderr, "add_array: cur = %d, id = %d\n", i, sfds[i]);
      }
   }
   /* Add the new descriptor to the end of the list */
   sfds[*count] = new_sfd;
   (*count)++;
}

/* Extend the buffer space to include data for a new socket */
void add_buf(int count)
{
   /* buffers for storing input data */
   ReadBuf = realloc(ReadBuf, sizeof(buf) * count);
   memset(ReadBuf[count - 1].buffer, 0x00, BS);
   ReadLen = realloc(ReadLen, sizeof(int) * count);
   /* buffers for storing output data */
   WritBuf = realloc(WritBuf, sizeof(buf) * count);
   memset(WritBuf[count - 1].buffer, 0x00, BS);
   WritLen = realloc(WritLen, sizeof(int) * count);
}


void rem_buf(int loc, int count)
{
   for (int i = loc; i < count; i++)
   {
      memcpy (ReadBuf + i, ReadBuf + i + 1, sizeof(buf));
      memcpy (WritBuf + i, WritBuf + i + 1, sizeof(buf));
      WritLen[i] = WritLen[i + 1];
      ReadLen[i] = ReadLen[i + 1];
   }
   
   WritLen[count - 1] = -1;
   ReadLen[count - 1] = -1;
}

int sub_array(int *sfds, int *count, int rem_sfd)
{
   int ret = -1;
   for (int i = 0; i < *count; i++)
   {
      if (sfds[i] == rem_sfd)
      {
         ret = i;
         sfds[i] = sfds[i + 1];
         sfds[i + 1] = rem_sfd;
      }
   }
   if (ret > -1)
   {
      //rem_buf(ret, *count);
      (*count)--;
   }
   return ret;
}

int get_loc(int sfd)
{
   int ret = -1;
   for (int i = 0; i < Count; i++)
   {
      if (Sfds[i] == sfd)
      {  
         ret = i;
      }
   }
   return ret;
}



void handle(int sfd, int op)
{
   int len, curlen;
   int hand = get_loc(sfd);
   char buff[BS];
   memset(buff, 0, BS); 
   
   switch(op)
   {
      case READ:
         len = read(sfd, buff, BS);
         if (len >= 0)
         { 
            curlen = strlen(ReadBuf[hand].buffer);
            memcpy(&(ReadBuf[hand].buffer[curlen]), buff, len); 
         }
         else
         {
            fprintf(stderr, "handle: ERNO\n");
            fprintf(stderr, "Errno: %s", strerror(errno));
         }
      break;
      
      case WRITE:
      break;
      
      default:
      break;
   }
   if (strstr(ReadBuf[hand].buffer, "GET: / HTTP1.1") != NULL)
      fprintf(stderr, "recieved that one thing");
}



int main(int argc, char *argv[])
{
   /* set signal handler for ctrl-c */
   set_sigint();

   /* open a socket with appropriate settings */
   int sfd = gen_socket();

   /* Setup the control structure for binding the socket to tcp */
   struct sockaddr_in6 addr;
   gen_addr(argc, argv, &addr);
   
   /* bind the socket to a port and print some stats */
   bind_socket(sfd, &addr); 

   /* Set listen on the main socket this is the one to accept from */
   listen(sfd, SOMAXCONN);

   /* set to nonblocking since from now on we need to do many things */
   fcntl(sfd, F_SETFL, O_NONBLOCK);

   /* setup for extendable array */
   int cap = 5;
   Sfds = malloc(cap * sizeof (int));



   /* Variables for the select */
   fd_set rfds, wfds, efds;
   struct timeval tv;
   int retval;
   
   unsigned int len = sizeof(struct sockaddr);
   //accept4(sfd, (struct sockaddr *)&addr, &len, SOCK_NONBLOCK);

   int ret;

   while (1)
   {
      /* Set up for each time select will be called */
      tv.tv_sec = 5;
      tv.tv_usec = 0;
   
      FD_ZERO(&rfds);
      FD_ZERO(&wfds);
      FD_ZERO(&efds);
      FD_SET(sfd, &rfds);
      for (int i = 0; i < Count; i++)
      {
         FD_SET(Sfds[i], &rfds);
         FD_SET(Sfds[i], &wfds);
         FD_SET(Sfds[i], &efds);
      }
   
      retval = select(sfd + Count + 1, &rfds, NULL, &efds, &tv);
      
      if (retval < 0)
      {
         fprintf(stderr, "Select error: %d\n", retval);
         fprintf(stderr, "Errno: %s", strerror(errno));
      }
      else if (retval)
      { 
         /* check all child sockets for input */
         for (int i = 0; i < Count; i++)
         {
            if (FD_ISSET(Sfds[i], &rfds))
            {
               handle(Sfds[i],READ);
            }
         }

         /* check the acceptance socket */
         fprintf(stderr, "select says somthing happened\n");
         if(FD_ISSET(sfd, &rfds))
         {
            fprintf(stderr, "incoming socket detected\n");
            ret = accept(sfd, (struct sockaddr *)&addr, &len);
            fcntl(ret, F_SETFL, O_NONBLOCK);
            fprintf(stderr, "Accepted %d from socket: %d\n", ret, sfd);
            add_array(Sfds, &Count, &cap, ret);
            add_buf(Count);
         }
      }
      else
      {
         fprintf(stderr, "Select: no data, just checking in\n");
      }
      

   
   }



   fprintf(stderr, "Server exited from main not sigint\n");
   return 0;
}




