#ifndef __LIST__H
#define __LIST__H
#define MAX 256
enum{NEXT, PREV};
enum{HEAD, TAIL};
typedef struct Node
{
  char ipAddr[MAX]; 
  short port;
  struct Node *ptr[2];
}Node;
typedef struct List
{
  Node *ptr[2];
}List;
void insert(List *, char *, short);
#endif
