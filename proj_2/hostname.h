#pragma once

#define MAX_URL_SIZE 2000
#define DEFAULT_USER "anonymous"
#define DEFAULT_PWD "default_pwd"

typedef enum{
    NO_USERNAME,
    USERNAME_ONLY,
    USERNAME_AND_PW,
    ERROR
} url_type;

typedef struct url_info{
  char host[MAX_URL_SIZE];
  char url_path[MAX_URL_SIZE]; 
  char user[MAX_URL_SIZE];
  char password[MAX_URL_SIZE];
} url_info_t;

char * get_ip(char *hostname);

char *build_url(char *hostname, char *user, char *password, char *url);

url_type parse_arguments(char *address, url_info_t * url_info);
