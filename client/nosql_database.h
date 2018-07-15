#pragma once
#include "../src/define.h"
#include "../src/link_list.h"
#include "../src/db_dict.h"
#include "../src/string_object.h"


typedef struct redis_db{
  int db_num;  // 数据库号
  int reference_index; // 指向该数据库的client数
  dict * dict;  // 一个字典对象用来保存键
}redis_db;

typedef struct redis_client{
  str querybuf;  // 输入缓冲
  str outbuf; // 输出缓冲
  int flag; // 标记
  str client_name; // 客户端姓名
  int sock; // 客户端对应描述符
  int argc; // 参数个数
  str **argv; // 命令参数
  redis_db * db;
}redis_client;

typedef struct redis_server{
  pid_t pid;
  int dbnum;
  redis_db * db;
  int port;
  list * redis_client;
}redis_server;

