#pragma  once
#include "define.h"

typedef struct listNode{
  struct listNode * prev;
  struct listNode * next;
  void * value;
}listNode;

typedef struct list{
  listNode * head;
  listNode * tail;
  unsigned long len;
  void *(*dup)(void *ptr);
  void (*free)(void *ptr);
  int (*match)(void *ptr, void *key);
}list;

list *listCreate(void);
void listRelease(list *list);
void listEmpty(list *list);
list *listAddNodeHead(list *list, void *value);
list *listAddNodeTail(list *list, void *value);
list *listInsertNode(list *list, listNode *old_node, void *value, int after);
void listDelNode(list *list, listNode *node);
listNode *listSearchKey(list *list, void *key);
listNode *listIndex(list *list, long index);
listNode * listIterator(list * list);
void listIterNext(listNode ** node);
void listIterPrev(listNode ** node);



