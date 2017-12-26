#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>


int main(int argc, char ** argv)
{
   int ret;
   struct addrinfo * res, * ptr;
   struct addrinfo hints = {
      AI_CANONNAME,
      PF_UNSPEC,
      SOCK_STREAM,
      0,
      0,
      NULL,
      NULL,
      NULL
   };
   
   if (argc != 2)
   {
      fprintf(stderr, "%s: usage: %s host | addr.\n", *argv, *argv);
      exit(1);
   }
   ret = getaddrinfo(argv[1], NULL, &hints, &res);
   if (ret)
   {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
      exit(1);
   }
   for (ptr = res; ptr; ptr = ptr->ai_next)
   {
      if (ptr->ai_canonname)
         fprintf(stdout, "Canonical name:\n%s\nAdresses:\n", ptr->ai_canonname);
      switch (ptr->ai_family) 
      {
         case AF_INET:
         {
            char dst[INET_ADDRSTRLEN];
            struct in_addr * src = &((struct sockaddr_in *) ptr->ai_addr)->sin_addr;
     
            if(!inet_ntop(AF_INET, (const void *) src, dst, sizeof(dst))) 
            {
               fprintf(stderr, "inet_ntop: %s\n", strerror(errno));
               break;
            }
            fprintf(stdout, "%s\n", dst);
            break;
         } 
         case AF_INET6:
         {
            char dst[INET6_ADDRSTRLEN];
            struct in6_addr * src = &((struct sockaddr_in6 *) ptr->ai_addr)->sin6_addr;
    
            if (!inet_ntop(AF_INET6, (const void *) src, dst, sizeof(dst))) 
            {
               fprintf(stderr, "inet_ntop: %s\n", strerror(errno));
               break;
            }
            fprintf(stdout, "%s\n", dst);
            break;
         }
         default:
            fprintf(stderr, "getaddrinfo: %s\n", strerror(EAFNOSUPPORT));
      }
   } 
   freeaddrinfo(res);
   exit(0);
}

