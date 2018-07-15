#include "deserialization.h" 

void print_deser(list * node){
  int len = node->len;
  listNode * cur = listIterator(node);
  int i;
  for(i = 0; i < len; i++){
    printf("%s ", cur->value);
    listIterNext(&cur);
  }
  printf("\n");
}
// 服务器用反序列化
void server_deser(int *argc, char *** argv, char *buf){
  list * list = listCreate();
  int ret = deserialization(buf, list);  
  if(ret < 0){
    return;
  }
  *argc = list->len;
  *argv = (char**)malloc(sizeof(char**));
  **argv = (char*)malloc(sizeof(char*) * (*argc));
  listNode * cur = listIterator(list);
  unsigned long i;
  int j;
  int len = strlen((char*)cur->value); 
    for(j = 0; j < len; j++){
      ((char*)cur->value)[j] = toupper(((char*)cur->value)[j]);
    }
  for( i = 0; i < list->len; i++ ){
    int len = strlen((char*)cur->value); 
    (*argv)[i] = (char*)malloc(sizeof(char) * len + 1);  
    memcpy((*argv)[i],cur->value,len);
    (*argv)[i][len] = '\0';
    cur = cur->next;
  }
  listRelease(list);
}

// 客户端用反序列化
int deserialization(char *buf, list * node){
  int len = (strlen(buf));
  int i ,index = 0, size = 0;
  if(len <= 0){
    printf("empty request\n");
  }
  if(buf[index++] == '*'){
    int tmp = 0;
     char num_buf[16] = { 0 }; 
     while(buf[index] != '\r'){
        num_buf[tmp++] = buf[index++];
     }
     index++;
     if(buf[index] != '\n'){
        printf("1 error request\n");
        return -1;
     }
     size = atoi(num_buf);
  }else{
    printf("2 error request\n");
    return -2;
  }
  index++;
  for(i = 0; i < size; i++){
    int count = 0;
    char num_buf[16] = { 0 };
    int tmp = 0;
    if(buf[index++] != '$'){
      printf("3 error request\n");
      return -3;
    }
    while(buf[index] != '\r'){
      num_buf[tmp++] = buf[index++];
    }
    index++;
    if(buf[index++] != '\n'){
      printf("4 error request\n");
      return -4;
    }
    count = atoi(num_buf);
    char * val = (char*)malloc(count + 1);
    memset(val, 0x00, count + 1);
    int j;
    for(j = 0; j < count; j++){
      val[j] = buf[index++];
    }
    listAddNodeTail(node, val);
    index++;
    index++;
  }
  return 1;
}
