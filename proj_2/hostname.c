#include "hostname.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

char *get_ip(char *hostname){
  struct hostent *h;
  if ((h = gethostbyname(hostname)) == NULL) {
    herror("gethostbyname");
    exit(1);
  }
  
  return inet_ntoa(*((struct in_addr *) h->h_addr));
}