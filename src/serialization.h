#pragma once
#include "define.h"
#include "link_list.h"

int jump_space(char **p);
int next_space(char *p);

list * ser_buf(char * buffer);
int serialization(list * node, char ** p);

