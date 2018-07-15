#pragma once
#include "define.h"

#define SDS_MAX_PREALLOC 1024

typedef struct String{
  int _len;  // 长度
  int _free; // 可用剩余空间
  char _buf[]; // 存放字符串的空间
}String;

typedef char *str;

// 长度
static inline ssize_t sdslen(const str s){
  struct String * sds = (String *)(s - (sizeof(struct String)));
  return sds->_len;
} 
// 求可用长度
static inline ssize_t sdsvail(const str s){
  struct String * sds = (String *)(s - (sizeof(struct String)));
  return sds->_free;
}

str sdsewlen(const void * init, ssize_t len);
str sdsnew(const char * init);
str sdsempty( void );
str sdsup(const str s);
void sdsfree(str s);
void sdsclear(str s);

str sdsMakeRoomFor(str s, size_t addlen);
str sdscatlen(str s,const void *t, size_t len);
str sdscat(str s, const char * t);
str sdscatsds(str s, const str t);
str sdscpy(str s, const char *t);
str strcpylen(str s, const char *t, size_t len);
str sdsgrowzero(str s, size_t len);
