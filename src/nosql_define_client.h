#pragma once
#include "nosql_object.h"
#include "define.h"
struct client{
  int _fd; // socket 描述符
  char * _name; // cilent名 可以为空
  int client_ip; // client 的 ip 地址
  int flag; // client 的标志
  redis_db * redis_db; // 指向 database 的指针
  char input_buffer[4096]; // 输入缓冲区
  char small_output_buffer[4096]; // 端指令输出缓冲区
  char * long_output_buffer; // 长指令输出缓冲区
};

