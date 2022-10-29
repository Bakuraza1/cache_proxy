#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "env.h"

void split(char *str, char *delimiter, char **arr);

void split(char *str, char *delimiter, char **arr)
{
    char *token = strtok(str, delimiter);
    int i = 0;
    while (token != NULL)
    {
        arr[i] = token;
        i++;
        token = strtok(NULL, delimiter);
    }
}

void replace_char(char *string, char replace, char new)
{
  int slen = strlen(string);
  for (int i = 0; i < slen; i++)
    if (string[i] == replace)
      string[i] = new;
}

long get_time(){
    time_t seconds;
    time(&seconds);
    return seconds;
}
