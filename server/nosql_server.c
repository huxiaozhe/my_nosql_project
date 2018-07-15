#include "nosql_server.h"

int start(uint16_t port,const char * ip){
  // 创建套接字描述符
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock <  0){
    perror("sock :");
    return -1;
  }
  struct sockaddr_in server_socket;
  bzero(&server_socket, (sizeof(struct sockaddr_in)));
  server_socket.sin_family = AF_INET;
  server_socket.sin_addr.s_addr = htons(INADDR_ANY);
  inet_addr(ip);
  server_socket.sin_port = htons(port); 
  // 绑定端口号
  if(bind(sock, (struct sockaddr *)&server_socket, sizeof(struct sockaddr_in)) != 0){
    perror("bind");
    close(sock);
    return -1;
  }
  if(listen(sock, 10) < 0){
    perror("listen");
    close(sock);
    return -2;
  }
  return sock;
}
// 处理连接请求
void process_connect(redis_server * server, int listen_fd, int epoll_fd){
  struct sockaddr_in client_addr;
  socklen_t len = sizeof(client_addr);
  int connect_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &len);
  if(connect_fd < 0){
    perror("accpet");
    return;
  }
  struct epoll_event ev;
  ev.data.fd = connect_fd;
  ev.events = EPOLLIN;
  // 添加到 epoll 监视中
  int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connect_fd, &ev);
  if(ret < 0){
    perror("epoll_ctl");
    return;
  }
  // 初始化一个 client 对象
  redis_client * client = (redis_client*)malloc(sizeof(* client));
  client->querybuf = (char*)malloc(4096);
  client->outbuf = (char*)malloc(4096);
  client->flag = -1;
  client->client_name = NULL;
  client->sock = connect_fd;
  client->argc = 0;
  client->argv = NULL;
  client->db = &(server->db[0]);
  listAddNodeHead(server->redis_client, client);
  server->dbnum++;
  return;
}
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
  memset(cur->querybuf, 0x00, 4096);
  cur->argc = 0;
  cur->argv = NULL;
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
 // for(i = 0; i < cur->argc; i++){
 //   printf("%s\n", cur->argv[i]);
 // }
 // // 现在 argc 保存参数个数 argv 保存具体参数
  generate_object(cur);
  list * list = ser_buf(cur->outbuf);
  char *p1 = NULL;
  int len = serialization(list, &p1);
  write(cur->sock, p1, len);
  free(p1);
  for(i = 0; i < cur->argc; i++){
    free(cur->argv[i]);
    cur->argv[i] = NULL;
  }
  free(*p);
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

int main(int argc, char**argv){
 struct redis_server server; 
  short port;
  const  char * ip;
  if(argc < 2){
    port = 8080;
    ip = "127.0.0.1";
  }else if(argc < 3){
    printf("error cin\n");
    return -1;
  }else if(argc < 4){
    ip = argv[1];
    port =htons(atoi(argv[2]));
  }else{
    return -2;
  }
  int ret;
  int sock = start(port, ip);
  server.pid = getpid();
  server.dbnum = 0;
  server.port = port;
  server.redis_client = listCreate();
  server_db_init(&server);
  int epoll_fd = epoll_create(10); 
  if(epoll_fd < 0){
    perror("epoll_create");
    return -3;
  }
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = sock;
  ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event);
  if(ret < 0){
    perror("epoll_ctl");
    return -4;
  }
  for(; ;){
    struct epoll_event events[16];
    int size = epoll_wait(epoll_fd, events, 16, -1);
    if( size < 0 ){
      perror("epoll_wait");
      continue;
    }
    if( size == 0 ){
      printf("epoll timeout\n");
      continue;
    }
    int i;
    for( i = 0 ; i < size; i++ ){
      if(!( events[i].events & EPOLLIN )){
        continue;
      }
      if(events[i].data.fd == sock){
        process_connect(&server, sock, epoll_fd);
        printf("connect ok\n");
      }else{
        process_listen(&server, events[i].data.fd, epoll_fd);
      }
    }
  }
  return 0;
}
