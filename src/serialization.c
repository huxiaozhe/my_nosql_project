#include "serialization.h" 


int jump_space(char **p){
  while((**p) == ' '){
    (*p)++;
  }
  if((**p) == '\0')
    return 0;
  return 1;
}
// 下一个空格的距离
int next_space(char *p){
  int result = 0;
  while(!isspace(*p) && *p != '\0'){
    p++;
    result++;
  }
  return result;
}

list * ser_buf(char * buffer){
  int len = strlen(buffer);
  char * p;
  p = buffer;
  jump_space(&p);
  if(len <= 0){
    return NULL;
  }
  list * re_list = listCreate();
  while(1){
    char * buf; 
    int size = next_space(p);
    if(size <= 0){
      break;
    }
    buf = (char*)malloc(size + 1);
    memset(buf, 0x00, size + 1);
    memcpy(buf, p, size);
    p += size;
    listAddNodeTail(re_list, buf);
    int flag = jump_space(&p);
    if(flag == 0)
      break;
  }
  return re_list;
}

int serialization(list * node, char **p){
  char buf[4096] = { 0 };
  int index = 4;
  listNode *cur;
  int k = 0;
  int len = node->len;
  long i;
  buf[0] = '*';
  buf[1] = '0' + len; 
  buf[2] = '\r';
  buf[3] = '\n';
  for(i = 0; i < len; i++){
    cur = listIndex(node, i);
    int str_len = strlen((char*)cur->value);
    buf[index++] = '$';
    int tmp = str_len;
    int j = 0;
    while(tmp){
      tmp /= 10;
      j++;
    }
    tmp = str_len;
    for(k = j - 1; k >= 0; k--){
      buf[index + k] = '0'+tmp%10;
      tmp /= 10;
    }
    index += j;
    buf[index++] = '\r';
    buf[index++] = '\n';
    memcpy(buf + index, cur->value, str_len);
    index += str_len;
    buf[index++] = '\r';
    buf[index++] = '\n';
  } 
  int l = strlen(buf);
  *p = (char*)malloc(l + 1);
  memset(*p, 0x00, l + 1);
  memcpy(*p, buf, l);
  return l;
}


