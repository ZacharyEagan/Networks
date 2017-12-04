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
#include "json-server.h"
#include "parser.h"



void server_shutdown()
{
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

   /* Variables for the select */
   fd_set rfds;
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
      FD_SET(sfd, &rfds);
   
      retval = select(sfd + 1, &rfds, NULL, NULL, &tv);
      
      if (retval < 0)
      {
         fprintf(stderr, "Select error: %d\n", retval);
         fprintf(stderr, "Errno: %s", strerror(errno));
      }
      else if (retval)
      {
         fprintf(stderr, "select says somthing happened\n");
         if(FD_ISSET(sfd, &rfds))
         {
            fprintf(stderr, "incoming socket detected\n");
            ret = accept(sfd, (struct sockaddr *)&addr, &len);
            fprintf(stderr, "Accepted %d from socket: %d\n", ret, sfd);
            
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




