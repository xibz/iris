#include "list.h"
#include <string.h>
#include <stdlib.h>
void insert(List *l, char *ipAddr, short port)
{
  Node *temp = malloc(sizeof(Node));
  strcpy(temp->ipAddr, ipAddr);
  temp->port = port;
  temp->ptr[NEXT] = NULL;
  temp->ptr[PREV] = l->ptr[TAIL];
  if(l->ptr[HEAD] == NULL)
    l->ptr[HEAD] = temp;
  else
  {
    l->ptr[TAIL]->ptr[NEXT] = temp;
    temp->ptr[PREV] = l->ptr[TAIL];
  }
  l->ptr[TAIL] = temp;
}
