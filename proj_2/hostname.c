#include "hostname.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_ip(char *hostname) {
  struct hostent *h;
  if ((h = gethostbyname(hostname)) == NULL) {
    herror("gethostbyname");
    exit(1);
  }

  return inet_ntoa(*((struct in_addr *) h->h_addr));
}

char *build_url(char *hostname, char *user, char *password, char *url) {
  sprintf(url, "ftp://%s:%s@%s", user, password, hostname);
  return url;
}

url_type parse_arguments(char *address, url_info_t * url_info) {
  char * host = url_info->host;
  char * url_path = url_info->url_path;
  char * user = url_info->user;
  char * password = url_info->password;

  if (sscanf(address, "ftp://%99[^:]:%99[^@]@%99[^/]/%99[^\n]", user, password, host, url_path) == 4) {
    printf("User and password\n");
    return USERNAME_AND_PW;
  }
  else if (sscanf(address, "ftp://%99[^@]@%99[^/]/%99[^\n]", user, host, url_path) == 3) {
    printf("User only\n");
    strcpy(password, DEFAULT_PWD);
    return USERNAME_ONLY;
  }
  else if (sscanf(address, "ftp://%99[^/]/%99[^\n]", host, url_path) == 2) {
    printf("Found no password nor user\n");
    strcpy(user, DEFAULT_USER);
    strcpy(password, DEFAULT_PWD);
    return NO_USERNAME;
  }
  printf("Error\n");
  return ERROR;
}
