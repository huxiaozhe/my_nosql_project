#pragma once
#include "../server/nosql_database.h"
#include "define.h"
#include "link_list.h"
#include "deserialization.h"
#include "serialization.h"
#include "skip_list.h"

// ************************ 
// 对象类型
#define NOSQL_STRING 1
#define NOSQL_LIST 2
#define NOSQL_DICT 3
#define NOSQL_ZSET 4

// ***********************
// ENCODING DEFINE
#define NOSQL_ENCODING_INT 5
#define NOSQL_ENCODING_STR 6
#define NOSQL_ENCODING_LINKLIST 7
#define NOSQL_ENCODING_HASH 8
#define NOSQL_ENCODING_SKIPLIST 9


typedef struct nosql_object{
  unsigned int type; // 类型
  unsigned int encoding; // 编码 底层实现的数据结构
  void * ptr;
}nosql_object;


nosql_object * string_operation(char ** out ,redis_client *);
nosql_object * zset_opernation(char ** out ,redis_client *);
nosql_object *  list_opernation(char ** out ,redis_client *);
nosql_object * hash_opernation(char ** out ,redis_client *);
nosql_object * del_opernation(char ** out ,redis_client *);

int generate_object(redis_client *);


