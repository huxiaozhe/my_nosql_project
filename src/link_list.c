#include "link_list.h"

int compare_key(void *key, void * value);

// 创建一个不包含任何节点的新链表
list *listCreate(void){
  struct list *list;
  if ((list = (struct list *)malloc(sizeof(*list))) == NULL)
    return NULL;
  list->head = list->tail = NULL;
  list->len = 0;
  return list;
}
// 从列表中删除所有元素，而不破坏列表本身
void listEmpty(list *list){
  unsigned long len;
  listNode *current, *next;

  current = list->head;
  len = list->len;
  while(len--) {
    next = current->next;
    if (list->free) list->free(current->value);
    free(current);
    current = next;
  }
  list->head = list->tail = NULL;
  list->len = 0;
}
// 释放给定链表
void listRelease(list *list){
  listEmpty(list);
  free(list);
}
// 头插
list *listAddNodeHead(list *list, void *value){
  listNode *node;

  if ((node = (listNode *)malloc(sizeof(*node))) == NULL)
    return NULL;
  node->value = value;
  if (list->len == 0) {
    list->head = list->tail = node;
    node->prev = node->next = NULL;
  } else {
    node->prev = NULL;
    node->next = list->head;
    list->head->prev = node;
    list->head = node;
  }
  list->len++;
  return list;
}
// 尾插
list *listAddNodeTail(list *list, void *value){
  listNode *node;

  if ((node = (listNode *)malloc(sizeof(*node))) == NULL)
    return NULL;
  node->value = value;
  if (list->len == 0) {
    list->head = list->tail = node;
    node->prev = node->next = NULL;
  } else {
    node->prev = list->tail;
    node->next = NULL;
    list->tail->next = node;
    list->tail = node;
  }
  list->len++;
  return list;
}
// 将一个给定值插入到给定节点的之前或之后
list *listInsertNode(list *list, listNode *old_node, void *value, int after){
  listNode *node;
  if ((node = (listNode *)malloc(sizeof(*node))) == NULL)
    return NULL;
  node->value = value;
  if (after) {
    node->prev = old_node;
    node->next = old_node->next;
    if (list->tail == old_node) {
      list->tail = node;
    }
  } else {
    node->next = old_node;
    node->prev = old_node->prev;
    if (list->head == old_node) {
      list->head = node;
    }
  }
  if (node->prev != NULL) {
    node->prev->next = node;
  }
  if (node->next != NULL) {
    node->next->prev = node;
  }
  list->len++;
  return list;
}
// 删除节点
void listDelNode(list *list, listNode *node){
  if (node->prev)
    node->prev->next = node->next;
  else
    list->head = node->next;
  if (node->next)
    node->next->prev = node->prev;
  else
    list->tail = node->prev;
  if (list->free) list->free(node->value);
  free(node);
  list->len--;

}
// 成功第一个匹配的节点指针返回(搜索从头部开始)
// 如果不存在匹配节点，则返回NULL
listNode *listSearchKey(list *list, void *key){
  listNode * itre;
  itre = list->head;
  while(itre){
    if(compare_key(key, itre->value)){
      return itre;
    } 
    itre = itre->next;
  }
  return NULL;
}
// 返回在指定的基于0的索引处的元素，其中0是head, 1是head旁边的元素
listNode *listIndex(list *list, long index){
  listNode *n;

  if (index < 0) {
    index = (-index)-1;
    n = list->tail;
    while(index-- && n) n = n->prev;
  } else {
    n = list->head;
    while(index-- && n) n = n->next;
  }
  return n;
}
int compare_key(void *key, void * value){
  return !strcmp((char*)key,(char*)value); 
}
listNode * listIterator(list * list){
  return list->head;
}
void listIterNext(listNode ** node){
  *node = (*node)->next;
}
void listIterPrev(listNode ** node){
  *node = (*node)->prev;
}

