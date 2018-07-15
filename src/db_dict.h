#pragma once
#include "define.h"
#include <stdlib.h>

#define DICT_ERR -1
#define DICT_OK 0
#define DICT_INSTALL_SIZE 4

typedef struct dict_entry{
  void * _key; // 键永远是一个字符对象
  void * val;
  // next 用于保存值
  struct dict_entry * _next; 
}dict_entry;

typedef struct db_dict{
  dict_entry ** dict_entry; // 哈希数组
  unsigned long size; // 大小
  unsigned long size_mask; // 掩码
  unsigned long used; // 使用
}db_dict;

typedef struct dict{
  void * privdata;  
  db_dict hash_table[2];
  int rehashindex;
  unsigned long iterators;
}dict;

#define dictIsRehashing(d) ((d)->rehashindex != -1)

typedef void (dictScanFunction)(void *privdata, const dict_entry *de);

uint64_t hash_function(dict *d, const void * key);
dict *dict_Create(void * privDataPtr);
int dictExpand(dict *d, unsigned long size);
dict_entry *dictAddRaw(dict *d, void *key, dict_entry **existing);
int dictAdd(dict *d, void *key, void *val);
dict_entry * dictFind(dict *d, const void *key);
void *dictFetchValue(dict *d, const void *key);
int dictDelete(dict *d, const void *key);
void dictRelease(dict *d);
int dictRehash(dict *d, int n);

