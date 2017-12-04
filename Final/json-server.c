#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

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



int main(int argc, char *argv[])
{

   struct sigaction sa;   
   sa.sa_handler = sigint_handler;
   sigfillset(&sa.sa_mask);
   sa.sa_flags = 0;
   if (-1 == sigaction(SIGINT, &sa, NULL))
   {
      perror("Couldn't set signal handler for SIGINT");
      return 2;
   }





      int sfd = socket(AF_INET6, SOCK_STREAM, 0);
      if (sfd <= 0)
      {
         fprintf(stderr, "Socket Creation Failed\n");
         fprintf(stderr, "Ernor: %s\n", strerror(errno));
         return -1;
      }
      fprintf(stderr, "Socket number: %d\n", sfd);
    
      int opt = 1;
      if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
      {
         fprintf(stderr, "Confusing sockets call\n");
         fprintf(stderr, "Errno: %s", strerror(errno));
         return -2;
      }

 
      struct sockaddr_in6 addr, foo;
      unsigned int len = sizeof(struct sockaddr);
      addr.sin6_family = AF_INET6;
      char map46[7+16+1];

      if (argc != 2)
      {
         addr.sin6_addr = in6addr_any;
      }
      else
      {
         if (!inet_pton(AF_INET6, argv[1], &(addr.sin6_addr)))
         {
            sprintf(map46, "::FFFF:%s", argv[1]);
            fprintf(stderr, "Not an IPV6 addr. Trying mapped v4: %s\n", map46);
            if (!inet_pton(AF_INET6, map46, &(addr.sin6_addr)))
            {
               fprintf(stderr, "inet_pton failed twice\n");
            }
         }
            
      }
      addr.sin6_port = htons(0);
      if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
      {
         fprintf(stderr, "Socket Bind Failure\n");
         fprintf(stderr, "Erno: %s\n", strerror(errno));
         return -3;
      }
      getsockname(sfd, (struct sockaddr *) &foo, &len);
      char straddr[INET6_ADDRSTRLEN];
      inet_ntop(AF_INET6, &foo.sin6_addr, straddr, sizeof(straddr));
      fprintf(stderr, "Socket number: %s port: %d\n", 
            straddr,
            ntohs(foo.sin6_port));
    

   printf ("HTTP server is using TCP port %d\n", ntohs(foo.sin6_port));
   printf ("HTTPS server is using TCP port -1\n");
   fflush(stdout);


   while (1)
   {
   
   }



   fprintf(stderr, "Server exited from main not sigint\n");


   return 0;
}




