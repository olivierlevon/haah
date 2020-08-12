/*

http://livre.g6.asso.fr/index.php/La_commande_haah_%28host-address-address-host%29

L'exemple proposé n'est autre qu'une sorte de nslookup (très) simplifié. 
Si par exemple on lui donne en argument une adresse numérique (IPv4 ou IPv6), 
il imprime le nom complètement qualifié correspondant lorsque la requête DNS aboutit.

L'extrait de session qui suit illustre l'utilisation de cette commande.

$ haah bernays
Canonical name:
bernays.ipv6.logique.jussieu.fr
Adresses:
2001:660:101:101:200:f8ff:fe31:17ec
3ffe:304:101:1:200:f8ff:fe31:17ec
$ haah 134.157.19.71
Canonical name:
bernays.logique.jussieu.fr
Adresses:
134.157.19.71
$

Compiler sur Windows (Windows Seven/Visual Studio 2017 Community)
cl /DWIN32 haah.c ws2_32.lib /link /SUBSYSTEM:CONSOLE /MACHINE:X86 

Compiler sur MacOS/Linux
gcc -Wall haah.c -o haah.o

*/

#undef UNICODE

#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32

#pragma comment(lib, "ws2_32.lib")

#define _CRT_SECURE_NO_WARNINGS	1
#define _CRT_SECURE_NO_DEPRECATE 1 

#define _WIN32_WINNT 0x0601

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>



#else

#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // struct in6_addr
#include <netinet/ip6.h> // struct ip6_hdr
#include <netdb.h>
#include <arpa/inet.h> // inet_ntop()
#include <netinet/icmp6.h>


 int IN6_IS_ADDR_GLOBAL(const struct in6_addr *addr)
{
      /* Normative reference: RFC4291 § 2.4 */
      return !(IN6_IS_ADDR_UNSPECIFIED(addr)
            || IN6_IS_ADDR_LOOPBACK(addr)
            || IN6_IS_ADDR_LINKLOCAL(addr)
            || IN6_IS_ADDR_MULTICAST(addr));
}

#endif

#define PROD_VERS "1.1"

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


//===========================================================================================================================
//	Usage
//===========================================================================================================================

 void Usage(void)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "HAAH host to address - address to host " PROD_VERS "\n");
	fprintf(stderr, "  haah -4/6 host-or-ip-address\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  -4           - ipv4\n");
	fprintf(stderr, "  -6           - ipv6\n");
	fprintf(stderr, "  -h[elp]      - Help\n");
	fprintf(stderr, "\n");
}


//===========================================================================================================================
//	ProcessArgs
//===========================================================================================================================

	char * name = "";

int ProcessArgs(int argc, char* argv[])
{	
	int	err =1;
	int	i;

			
	if ((argc <= 1) || (argc > 3))
	{
		Usage();
		err = 0;
		goto exit;
	}

	for (i = 1; i < argc; ++i)
	{
		if (strcmp( argv[i], "-4") == 0)
		{
			hints.ai_family = AF_INET;
		}
		else 
			if (strcmp(argv[i], "-6") == 0)
			{
				hints.ai_family = AF_INET6;
			}
			else
				if ((strcmp( argv[i], "-help") == 0) || 
					(strcmp( argv[i], "-h") == 0))
				{
					// Help
					Usage();
					err = 0;
					goto exit;
				}
				else
				{
					name = argv[i];
				}
	}
	if (*name == '\0')
	{
		name = "localhost";  // gethostname plutot gethostname(name, namelen);
	}
	
exit:
	return err;
}


int main(int argc, char **argv)
{
   int ret;
#ifdef WIN32
   WSADATA		wsd;
   HRESULT		err;
#endif
   struct addrinfo *res, *ptr;
 	
   if (!ProcessArgs(argc, argv))
	   return 1;

#ifdef WIN32
   err = WSAStartup(MAKEWORD(2, 2), &wsd);
#endif

   ret = getaddrinfo(name, NULL, &hints, &res);
   if (ret) 
   {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
      return 1;
   }
   for (ptr = res; ptr; ptr = ptr->ai_next) 
   {
      if (ptr->ai_canonname)
         fprintf(stdout, "Canonical name:\n%s\nAddresses:\n", ptr->ai_canonname);
         
      switch (ptr->ai_family) 
      {
         case AF_INET:
         {
            char dst[INET_ADDRSTRLEN];
            struct in_addr *src = &((struct sockaddr_in *) ptr->ai_addr)->sin_addr;
     
            if (!inet_ntop(AF_INET, (const void *) src, dst, sizeof(dst))) 
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
            struct in6_addr *src = &((struct sockaddr_in6 *)ptr->ai_addr)->sin6_addr;
    
            if (!inet_ntop(AF_INET6, (const void *) src, dst, sizeof(dst))) 
            {
               fprintf(stderr, "inet_ntop: %s\n", strerror(errno));
               break;
            }
			if (IN6_IS_ADDR_GLOBAL(src))
				fprintf(stdout, "%s (%d)\n", dst, ((struct sockaddr_in6 *)ptr->ai_addr)->sin6_scope_id); // pas le scope id
            else
				fprintf(stdout, "%s%%%d\n", dst, ((struct sockaddr_in6 *)ptr->ai_addr)->sin6_scope_id);

            break;
         }

         default:
            fprintf(stderr, "getaddrinfo: %s\n", strerror(EAFNOSUPPORT));
      }
   } 
   freeaddrinfo(res);

#ifdef WIN32
	WSACleanup();
#endif

   return 0;
}