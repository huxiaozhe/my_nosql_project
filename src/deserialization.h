#pragma once
#include "define.h"
#include "link_list.h"

void print_deser(list * node);


int deserialization(char *buf, list *);

void server_deser(int * argc, char *** argv, char *buf);
