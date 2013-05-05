#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "list.h"
#include <pthread.h>
void runServer(char *);

void runServer(char *portStr)
{
  List l = {NULL, NULL};
}

int main(int argc, char *argv[])
{
  List l;
  l.ptr[HEAD] = NULL;
  l.ptr[TAIL] = NULL;
  int i;
  for(i = 0; i < 5; ++i)
  {
    insert(&l, "here", i);
  }
  Node *temp = l.ptr[HEAD];
  while(temp != NULL)
  {
    printf("%d\n", temp->port);
    temp = temp->ptr[NEXT];
  }
  return 0;
}
