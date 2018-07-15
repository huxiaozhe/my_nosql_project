#include "nosql_client.h"

void print_prompt(){printf("192.163.122.1 > ");}

int start(){
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0){
    perror("socket");
    return -1;
  }
  struct sockaddr_in client_addr;
  bzero(&client_addr, sizeof(client_addr));
  client_addr.sin_family = AF_INET;
  inet_pton(AF_INET, "192.168.122.1", &client_addr.sin_addr);
  client_addr.sin_port = htons(8080);

  int ret = connect(sock, (struct sockaddr*)&client_addr, sizeof(client_addr));
  if(ret < 0){
    perror("connect");
    return -2;
  }
  return sock;
}

int main( void ){
  char input_buffer[4096] = { 0 };
  char output_buffer[4096] = { 0 };
  int sock = start();
  if(sock < 0){
    return -2;
  }
  printf("connect ok\n");
  while(1){
    print_prompt();
    memset(input_buffer, 0x00, 4096);
    memset(output_buffer, 0x00, 4096);
    scanf("%[^\n]%*c", input_buffer);
    char * p = NULL;
    list * node = ser_buf(input_buffer);
    int len = serialization(node , &p);    
    printf("%s\n", p);
    listEmpty(node);
    int ret = 0;
    ret = write(sock, p, len); 
    free(p);
    if(ret < 0){
      perror("write");
      continue;
    }
    ret = read(sock, output_buffer, 4096);
    if(ret < 0){
      perror("read");
      return 0;
    }
    if(ret == 0){
      printf("server stop\n");
      return 0;
    }
    deserialization(output_buffer, node);
    print_deser(node); 
    listRelease(node);
  }  
  return 0;
}
