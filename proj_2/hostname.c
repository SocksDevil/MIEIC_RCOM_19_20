#include "hostname.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USERNAME_URL "ftp://(([a-z0-9]+):([a-z0-9]+)@)*([\\.a-z0-9]+)/([\\./a-z0-9]*)$"
#define NO_USERNAME_URL "ftp://([\\.a-z0-9]+)/([\\./a-z0-9]+)$"

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

url_type parse_arguments(char *address, char *host, char *url_path, char *user, char *password) {
  regex_t non_username_url, username_url;
  if (regcomp(&non_username_url, NO_USERNAME_URL, REG_EXTENDED) != 0) {
    printf("Could not compile non username regex!\n");
  }
  if (regcomp(&username_url, USERNAME_URL, REG_EXTENDED) != 0) {
    printf("Could not compile username regex!\n");
  }

  if (!regexec(&non_username_url, address, 0, NULL, 0)) {
    printf("Found non username url!\n");
    int host_length = (int)(strchr(address + 6, '/') - (address + 6));
    memcpy(host, address + 6, host_length);
    host[host_length] = '\0';
    strcpy(url_path, address + 7 + host_length);
    return NO_USERNAME;
  }
  else if (!regexec(&username_url, address, 0, NULL, 0)) {
    int username_length = (int) (strchr(address + 6, ':') - (address + 6));
    memcpy(user, address + 6, username_length);
    user[username_length] = '\0';
    int password_length = (int) (
      strchr(address + 6 + username_length, '@') - (address + 6 + username_length)) - 1;
    memcpy(password, address + 7 + username_length, password_length);
    password[password_length] = '\0';
    int host_length = (int) (strchr(address + 8 + username_length + password_length, '/') -
                             (address + 8 + username_length + password_length));
    memcpy(host, address + 8 + username_length + password_length, host_length);
    host[host_length] = '\0';
    int url_path_index = (int)(strchr(address + 6, '/') - (address + 6));
    strcpy(url_path, address + url_path_index + 7);
    return USERNAME;
  }
  else {
    printf("Failed to match which kind of url it is\n");
    return ERROR;
  }
}
