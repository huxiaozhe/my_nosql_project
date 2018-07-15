#include "string_object.h"

str sdsnewlen(const void * init, ssize_t len){
  String * result;
  if(init){
    result = (String *)malloc(sizeof(struct String) + len + 1);
  }else{
    result = (String *)calloc(sizeof(struct String) + len + 1, 1);
  }
  if(result == NULL){
    return NULL;
  }
  result->_len = len;
  result->_free = 0;
  if(init && len){
    memcpy(result->_buf, init, len);
    result->_buf[len] = '\0';
  }
  return (char*)result->_buf;
}
// 创建字符串
str sdsnew(const char * init){
  size_t initlen = (init == NULL) ? 0 : strlen(init);
  return sdsnewlen(init, initlen);
}
// 创建空字符串对象
str sdsempty( void ){
  return sdsnewlen("", 0);
}
// copy a new object
str sdsup(const str s){
  return sdsnewlen(s, sdslen(s));
}
// 释放指定的 str
void sdsfree(str s){
  if(s == NULL) return;
  free(s - sizeof(struct String));
}
// 清空字符串
void sdsclear(str s){
  struct String * sh = (String *)(s -(sizeof(struct String)));
  sh->_free += sh->_len;
  sh->_len = 0;
  sh->_buf[0] = '\0';
}
// 追加指定字符串
//**************************
// 扩大sds字符串末尾的空闲空间，使调用者确信在调用此函数之后
// 可以在字符串结束后覆盖到addlen字节，再加上一个字节作为nul术语。
// 注意:这不会改变sdslen()返回的sds字符串的长度
// 但是只有我们拥有的空闲缓冲区空间
str sdsMakeRoomFor(str s, size_t addlen){
  struct String *sh, *newsh;
  size_t free_len = sdsvail(s);
  size_t len, new_len;
  if(free_len >= addlen) return s; // 剩余空间足够
  len = sdslen(s);
  sh = (String *)(s - (sizeof(struct String)));
  new_len = len + addlen;
  if(new_len < SDS_MAX_PREALLOC)
    new_len *= 2;
  else
    new_len += SDS_MAX_PREALLOC;
  // 扩容 生成新的 string 对象
  newsh = (String *)realloc(sh, sizeof(struct String) + new_len + 1);
  if(newsh == NULL) return NULL;
  newsh->_free = new_len - len;
  return newsh->_buf;
}
// 将由长度为 len 的字符串 t 指向的指定二进制安全字符串追加到指定的 sds 字符串 s 的末尾
// 调用之后，传递的 sds 字符串不再有效，所有引用必须替换为调用返回的新指针
str sdscatlen(str s,const void *t, size_t len){
  struct String * sh;
  size_t cur_len = sdslen(s);
  s = sdsMakeRoomFor(s, len);
  sh = (struct String *)(s - (sizeof(struct String)));
  memcpy(s + cur_len, t, len);
  sh->_len = cur_len + len;
  sh->_free = sh->_free - sh->_len;
  s[cur_len + len] = '\0';
  return 0;
}
// 将指定的以 '\0' 结尾的 C 字符串附加到 sds 字符串 s 中
// 调用之后，传递的 sds 字符串不再有效，所有引用必须替换为调用返回的新指针
str sdscat(str s, const char * t){
  return sdscatlen(s, t, strlen(t));
}
// 将给定 string 字符串拼接到另一个 string 字符串的末尾
str sdscatsds(str s, const str t){
  return sdscatlen(s, t, sdslen(t));
}
// 破坏性地修改 str 字符串 s 
// 来保存指定的长度为 len 字节 t 的二进制安全字符串。
str strcpylen(str s, const char *t, size_t len){
  struct String *sh = (struct String*) (s-(sizeof(struct String)));
  size_t totlen = sh->_free+sh->_len;
  if (totlen < len) {
    s = sdsMakeRoomFor(s,len-sh->_len);
    if (s == NULL) return NULL;
    sh = (struct String*) (s-(sizeof(struct String)));
    totlen = sh->_free+sh->_len;
  }
  memcpy(s, t, len);
  s[len] = '\0';
  sh->_len = len;
  sh->_free = totlen-len;
  return s;
}
// 与sdscpylen()类似，但 t 必须是一个'\0'终止字符串
// 以便使用strlen()获取字符串的长度
str sdscpy(str s, const char *t){
  return strcpylen(s, t, strlen(t));
}

// 使sds具有指定的长度
// 不属于 sds 原始长度的字节将被设置为零
// 如果指定长度小于当前长度，则不执行操作
str sdsgrowzero(str s, size_t len){
  struct  String *sh = (struct String*)(s-(sizeof(struct  String)));
  size_t totlen, curlen = sh->_len;

  if (len <= curlen) return s;
  s = sdsMakeRoomFor(s,len-curlen);
  if (s == NULL) return NULL;
  /* Make sure added region doesn't contain garbage */
  sh = (struct String*)(s-(sizeof(struct  String)));
  memset(s+curlen,0,(len-curlen+1)); /* also set trailing \0 byte */
  totlen = sh->_len+sh->_free;
  sh->_len = len;
  sh->_free = totlen-sh->_len;
  return s;
}
