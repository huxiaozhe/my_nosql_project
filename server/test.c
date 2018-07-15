#include "nosql_server.h"


// 处理来自客户端的请求
void process_listen(redis_server * server, int connect_fd, int epoll_fd){
  int i;
  // 找出对应的客户端节点
  listNode * node = listIterator(server->redis_client);
  redis_client * cur;
  while(node){
    cur = (redis_client*)node->value;
    if(cur->sock == connect_fd){
      break;
    }
    listIterNext(&node);
  }
  if(node == NULL){
    return;
  }
  // 从 socket 中读取数据
  ssize_t read_size = read(connect_fd, cur->querybuf, 4095);
  if(read_size < 0){
    perror("read");
    return;
  }
  if(read_size == 0){
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connect_fd, NULL);
    printf("client del\n");
    return ;
  }
  // 处理序列化后的数据 
  char ** p = NULL;
  p = (char**)malloc(sizeof(char**));
  server_deser(&cur->argc, &p, cur->querybuf);
  cur->argv = p;
  
  // 现在 argc 保存参数个数 argv 保存具体参数
  generate_object(cur);
  list * list = ser_buf(cur->outbuf);
  char *p1 = NULL;
  int len = serialization(list, &p1);
  write(cur->sock, p1, len);
  for(i = 0; i < cur->argc; i++){
    free(p[i]);
  }
  free(p);
}


void server_db_init(redis_server * server){
  int i;
  server->db = (redis_db*)malloc(4 * sizeof(struct redis_db));
  for(i = 0; i < 4; i++){
      server->db[i].db_num = i;
      server->db[i].reference_index = 0;
      server->db[i].dict = dict_Create(NULL);
  }
}

int main( void ){
  struct redis_server server; 
  server.pid = getpid();
  server.dbnum = 0;
  server.port = 0;
  server.redis_client = listCreate();
  server_db_init(&server);
  redis_client * client = (redis_client*)malloc(sizeof(* client));
  client->querybuf = (char*)malloc(4096);
  client->outbuf = (char*)malloc(4096);
  client->flag = -1;
  client->client_name = NULL;
  client->sock = 0;
  client->argc = 0;
  client->argv = NULL;
  client->db = &server.db[0];
  listAddNodeHead(server.redis_client, client);
  server.dbnum++;
  char * val = (char*)malloc(4);
  char * key = (char*)malloc(4);
  
  str value = sdsnew("value");
  memcpy(key, "key", 4);
  memcpy(val, "val", 4);

  nosql_object * res = (nosql_object *)malloc(sizeof(*res)); 
  res->type = NOSQL_STRING;
  res->encoding = NOSQL_ENCODING_STR;
  res->ptr = value;
  dictAdd(server.db[0].dict, key, res); 

  dict_entry * entry = dictFind(server.db[0].dict, key);
  nosql_object * in = (nosql_object *)entry->val;
  printf("%s\n", in->ptr);

  dict * d = dict_Create(NULL);
  dictAdd(d, key, value);
  dict_entry * en = dictFind(d, key);
  printf("%s\n", en->val);

  return 0;
}
