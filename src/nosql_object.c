#include "nosql_object.h"

int generate_object(redis_client * client){
  nosql_object * result;
  char ** out = NULL;
  out = (char**)malloc(sizeof(char**));
  switch((client->argv)[0][0]){
  case 'S':
    result = string_operation(out ,client); 
    break;
  case 'Z':
    result = zset_opernation(out ,client);
    break;
  case 'H':
    result = hash_opernation(out ,client);
    break;
  case 'L':
    result = list_opernation(out ,client);
    break;
  case 'R':
    result = list_opernation(out ,client);
    break;
  case 'G':
    result = string_operation(out ,client); 
    break;
  case 'D':
    result = del_opernation(out ,client);
    break;
  default:
    result = NULL;
    break;
  }
  client->outbuf = *out;
  if(result == NULL){
    return 0;
  }else{
    dictAdd(client->db->dict, client->argv[1], result);
    //printf("dictadd ---- %d %lu\n", ((zskiplist *)result->ptr)->level, ((zskiplist *)result->ptr)->length);
    return 1;
  }
  return 2;
}
// 字符串操作
nosql_object* string_operation(char ** out, redis_client * client){
  nosql_object * result = NULL;
  int len = client->argc;
  // 如果是 SET 操作 参数必须为三个
  if(strcmp(client->argv[0], "SET") == 0){
    if(len != 3){
      *out = (char*)(malloc(18));
      memcpy(*out, "SET COMMAND ERROR", 17);
      *out[17] = '\0';
      return result;
    }
    dict_entry * dict = dictFind(client->db->dict, client->argv[1]);
    // 如果没有该对象存在,则新建一个object
    if(dict == NULL){
      result = (nosql_object*)malloc(sizeof(*result));  
      result->type = NOSQL_STRING;
      result->encoding = NOSQL_ENCODING_STR;
      str s = sdsnew(client->argv[2]); 
      result->ptr = s;
      *out = (char*)malloc(3);
      memcpy(*out, "OK", 2);
      *out[2] = '\0';
      return result;
    }else{
      // 如果对象存在,则进行替换
      sdsfree((str)dict->val);
      dict->val = sdsnew(client->argv[2]);
      *out = (char*)malloc(3);
      memcpy(*out, "OK", 2);
      *out[2] = '\0';
      return result;
    }
    // 如果是 SLEN 操作 则参数必须为 2 个
  }else if(strcmp(client->argv[0], "SLEN") == 0){
    if(len != 2){
      *out = (char*)(malloc(18));
      memcpy(*out, "SET COMMAND ERROR", 17);
      *out[17] = '\0';
      return result;
    }
    dict_entry * dict = dictFind(client->db->dict, client->argv[1]);
    // 如果键不存在 返回 ERROR
    if(dict == NULL){
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }else{ // 存在则取出元素长度
      int len = sdslen((str)dict->val);
      int len2 = len;
      int i = 0;
      while(len2){
        len2 /= 10;
        i++;
      }
      *out = (char*)malloc(i + 1);
      sprintf(*out, "%d", len);
      *out[i] = '\0';
    }
    // 如果是 GET 操作,参数长度为 2
  }else if(strcmp(client->argv[0], "GET") == 0){
    if(len != 2){
      *out = (char*)(malloc(18));
      memcpy(*out, "GET COMMAND ERROR", 18);
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){ // 键不存在
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }else{ // 键存在
      nosql_object * obj = (nosql_object *)(entry->val);
      printf("%s\n", obj->ptr);
      int len = strlen((char*)obj->ptr);
      *out = (char*)malloc(len + 1);
      printf("%s\n", obj->ptr);
      memcpy(*out, obj->ptr, len);
      *out[len] = '\0';
      return 0;
    }
  }else{ // 指令错误 
    int sz = strlen((char*)client->argv[0]);
    *out = (char*)malloc(22 + sz + 1);
    memcpy(*out, "Error Unknow Command ", 22);
    memcpy(*out+22, client->argv[0], sz);
    return result;
  }
  return 0;
}

nosql_object * zset_opernation(char **out ,redis_client * client ){
  int i;
  nosql_object * result = NULL;
  int len = client->argc;
  if(strcmp(client->argv[0], "ZADD") == 0){
    if(len != 4){
      *out = (char*)(malloc(19));
      memcpy(*out, "ZADD COMMAND ERROR", 18);
      *out[18] = '\0';
      return result;
    }
    zskiplist * list = NULL;
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    // 不存在则创建新的有序集合对象
    if(entry == NULL){
      result = (nosql_object *)malloc(sizeof(*result));
      result->encoding = NOSQL_ENCODING_SKIPLIST;
      result->type = NOSQL_ZSET;
      list = zslCreate();
      printf("%d %lu\n",list->level, list->length);
      result->ptr = list;
      //  printf("%lu\n", list->length);
    } else {  // 对象存在
      result = NULL;
      nosql_object * obj = (nosql_object *)entry->val;
      list = (zskiplist *)obj->ptr;
      //printf("%d %lu\n",list->level, list->length);
    }
    int l = strlen(client->argv[3]); 
    //printf("%d %lu\n",list->level, list->length);
    //printf("%d %lu\n", ((zskiplist*)result->ptr)->level, ((zskiplist*)result->ptr)->length);
    // 参数类型判断 score 只能为 double
    for(i = 0; i < l; i++){
      if(isdigit(client->argv[3][i]) || client->argv[3][i] == '.'){
      }else{
        *out = (char*)(malloc(19));
        memcpy(*out, "ZADD COMMAND ERROR", 18);
        if(result)
          free(result);
        result = NULL;
        *out[18] = '\0';
        return result;
      }
    }
    double score = atof(client->argv[3]);
    str ses = sdsnew(client->argv[2]);
    zskiplistNode * node = zslInsert(list, score, ses);
    printf(" score %f ses %s\n", score, ses);
    // 插入失败
    if(node == NULL){
      *out = (char*)malloc(12);
      memcpy(*out, "NOT SUCCESS", 12);
      if(result)
        free(result);
      result = NULL;
      return result;
    }
    *out = (char*)malloc(3);
    memcpy(*out, "OK", 3);
    //printf("%d %lu\n", ((zskiplist*)result->ptr)->level, ((zskiplist*)result->ptr)->length);
    return result;
  }else if(strcmp(client->argv[0], "ZCARD") == 0){ // 获取包含节点数量
    if(len != 2){
      *out = (char*)(malloc(20));
      memcpy(*out, "ZCARD COMMAND ERROR", 19);
      *out[18] = '\0';
      return result;
    } // 查找该成员
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    } // 指针强转
    nosql_object * obj = (nosql_object*)entry->val;
    zskiplist * list = (zskiplist *)obj->ptr;
    unsigned long tmp = list->length;
    printf("%lu\n", tmp);
    int index = tmp;
    int count = 0;
    while(index){
      count++;
      index /= 10;
    }
    *out = (char*)malloc(count + 1);
    memset(*out, 0x00, count + 1);
    sprintf(*out, "%lu", tmp);
    return result;
  }else if(strcmp(client->argv[0], "ZCOUNT") == 0){
    if(len != 4){
      *out = (char*)(malloc(21));
      memcpy(*out, "ZCOUNT COMMAND ERROR", 20);
      *out[20] = '\0';
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    } // 指针强转
    nosql_object * obj = (nosql_object*)entry->val;
    zskiplist * list = (zskiplist *)obj->ptr;
    //printf("list level %d length %lu\n",list->level, list->length);
    zrangespec * range = (zrangespec*)malloc(sizeof(*range));
    range->min = atof(client->argv[2]); // 最小值
    range->max = atof(client->argv[3]); // 最大值
    range->maxex = 0;
    range->minex = 0;
    printf("range -- %f %f\n", range->min, range->max);
    int count = 0;
    // 分值范围内第一个出现的节点
    zskiplistNode * node = zslFirstInRange(list, range);
    printf("node %f %s\n", node->score, node->ses);
    if(node == NULL){
      *out = (char*)malloc(2);
      memcpy(out, "0", 2);
      return result;
    }
    // 遍历节点
    while(node != NULL && node->score <= range->max){
      count++;
      node = node->level[0].forward;
    }
    printf("count = %d\n", count);
    int j = 0;
    int tmp = count;
    while(tmp > 0){
      tmp /= 10;
      j++;
    }
    //printf("%d\n", j);
    // 数字转字符串
    *out = (char*)malloc(j + 1);
    memset(*out, 0x00, j + 1);
    sprintf(*out ,"%d", count);
    return result;
  }else if(strcmp(client->argv[0], "ZRANGE") == 0){ // zrange 方法 给定索引范围内的所有所有元素
    if(len != 4){
      *out = (char*)(malloc(21));
      memcpy(*out, "ZRANGE COMMAND ERROR", 20);
      *out[20] = '\0';
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
    // 取出跳表节点及范围
    nosql_object * obj = (nosql_object*)entry->val;
    zskiplist * list = (zskiplist *)obj->ptr;
    unsigned long min_range,max_range;
    min_range = atol(client->argv[2]);
    max_range = atol(client->argv[3]);
    // 参数判断
    if(list->length < min_range){
      *out = (char*)malloc(5);
      memcpy(*out, "NULL", 5);
      return result;
    }
    if(max_range > list->length){
      max_range = list->length;
    }
    printf("%lu %lu\n", min_range, max_range);
    zskiplistNode * node = zslGetElementByRank(list, min_range);
    unsigned long i;
    str s = sdsempty();
    // 拼接字符串
    for(i = 0; i <= max_range-min_range; i++){
      sdscat(s, node->ses);
      sdscat(s, " ");
      node = node->level[0].forward;
    }
    printf("%s\n", s);
    *out = (char*)malloc(sdslen(s) + 1);
    memcpy(*out, s, sdslen(s) + 1);
    sdsfree(s);
    return result;
  }else if(strcmp(client->argv[0], "ZRANK") == 0){ // 指定节点的排位
    if(len != 3){
      *out = (char*)(malloc(20));
      memcpy(*out, "ZRANK COMMAND ERROR", 19);
      *out[19] = '\0';
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
    // 取出跳表节点及范围
    nosql_object * obj = (nosql_object*)entry->val;
    zskiplist * list = (zskiplist *)obj->ptr;
    unsigned long i = zslGetRank(list, client->argv[2]);
    int tmp = i;
    int j = 0;
    while(tmp){
      tmp /= 10;
      j++;
    }
    // 数字转字符串
    *out = (char*)malloc(j + 1);
    memset(out, 0x00, j + 1);
    sprintf(*out ,"%lu", i);
    return result;
  }else if(strcmp(client->argv[0], "ZREM") == 0){
    if(len != 3){
      *out = (char*)(malloc(19));
      memcpy(*out, "ZREM COMMAND ERROR", 18);
      *out[18] = '\0';
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
    nosql_object * obj = (nosql_object*)entry->val;
    zskiplist * list = (zskiplist *)obj->ptr;
    unsigned long i = zslGetRank(list, client->argv[2]);
    unsigned long j = zslDeleteRangeByRank(list, i , i);
    if(j == 0){
      *out = (char*)malloc(5);
      memcpy(*out, "NULL", 5);
      return result;
    }
    *out = (char*)malloc(3);
    memcpy(*out, "OK", 3);
    return result;

  }else if(strcmp(client->argv[0], "ZSCORE") == 0){
    if(len != 3){
      *out = (char*)(malloc(21));
      memcpy(*out, "ZSCORE COMMAND ERROR", 20);
      *out[21] = '\0';
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
    nosql_object * obj = (nosql_object*)entry->val;
    zskiplist * list = (zskiplist *)obj->ptr;
    unsigned long i = zslGetRank(list, client->argv[2]);
    zskiplistNode * node = zslGetElementByRank(list, i);
    if(node == NULL){
      *out = (char*)malloc(5);
      memcpy(*out, "NULL", 5);
      return result;
    }
    char buf[64] = {0};
    sprintf(buf, "%f", node->score);
    int ln = strlen(buf);
    *out = (char*)malloc(ln + 1);
    memcpy(*out,  buf , len + 1);
    return result;
  }else{
    int sz = strlen((char*)client->argv[0]);
    *out = (char*)malloc(22 + sz + 1);
    memcpy(*out, "Error Unknow Command ", 22);
    memcpy(*out+22, client->argv[0], sz);
    return result;
  }
  return result;
}
nosql_object * list_opernation(char **out ,redis_client * client ){
  nosql_object * result = NULL;
  int len = client->argc;
  if(strcmp(client->argv[0], "LPUSH") == 0){
    if(len != 3){
      *out = (char*)(malloc(20));
      memcpy(*out, "LPUSH COMMAND ERROR", 20);
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
  }else if(strcmp(client->argv[0], "RPUSH") == 0){
    if(len != 3){
      *out = (char*)(malloc(20));
      memcpy(*out, "RPUSH COMMAND ERROR", 20);
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
  }else if(strcmp(client->argv[0], "LPOP") == 0){
    if(len != 2){
      *out = (char*)(malloc(19));
      memcpy(*out, "LPOP COMMAND ERROR", 19);
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
  }else if(strcmp(client->argv[0], "RPOP") == 0){
    if(len != 2){
      *out = (char*)(malloc(19));
      memcpy(*out, "ZSCORE COMMAND ERROR", 19);
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
  }else if(strcmp(client->argv[0], "LINDEX") == 0){
    if(len != 3){
      *out = (char*)(malloc(21));
      memcpy(*out, "LINDEX COMMAND ERROR", 21);
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
  }else if(strcmp(client->argv[0], "LLEN") == 0){
    if(len != 3){
      *out = (char*)(malloc(19));
      memcpy(*out, "LLEN COMMAND ERROR", 19);
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
  }else if(strcmp(client->argv[0], "LREM") == 0){
    if(len != 3){
      *out = (char*)(malloc(19));
      memcpy(*out, "LREM COMMAND ERROR", 19);
      *out[21] = '\0';
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
  }else{
    int sz = strlen((char*)client->argv[0]);
    *out = (char*)malloc(22 + sz + 1);
    memcpy(*out, "Error Unknow Command ", 22);
    memcpy(*out+22, client->argv[0], sz);
    return result;
  }

  return result;
}
nosql_object * hash_opernation(char **out ,redis_client * client ){
  nosql_object * result = NULL;
  int len = client->argc;
  if(strcmp(client->argv[0], "HSET") == 0){
    if(len != 4){
      *out = (char*)(malloc(19));
      memcpy(*out, "HSET COMMAND ERROR", 19);
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }

  }else if(strcmp(client->argv[0], "HGET") == 0){
    if(len != 3){
      *out = (char*)(malloc(19));
      memcpy(*out, "HGET COMMAND ERROR", 19);
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
  }else if(strcmp(client->argv[0], "HEXISTS") == 0){
    if(len != 3){
      *out = (char*)(malloc(19));
      memcpy(*out, "HGET COMMAND ERROR", 19);
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
  }else if(strcmp(client->argv[0], "HDEL") == 0){
    if(len != 3){
      *out = (char*)(malloc(19));
      memcpy(*out, "HDEL COMMAND ERROR", 19);
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
  }else if(strcmp(client->argv[0], "HLEN") == 0){
    if(len != 3){
      *out = (char*)(malloc(19));
      memcpy(*out, "HLEN COMMAND ERROR", 19);
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(20);
      memcpy(*out, "That Key Not Exists", 20);
      return result;
    }
  } else{
    int sz = strlen((char*)client->argv[0]);
    *out = (char*)malloc(22 + sz + 1);
    memcpy(*out, "Error Unknow Command ", 22);
    memcpy(*out+22, client->argv[0], sz);
    return result;
  }

  return result;
}
// --- del
nosql_object* del_opernation(char **out ,redis_client * client ){
  nosql_object * result = NULL;
  int len = client->argc;
  if(strcmp(client->argv[0], "DEL") == 0){
    if(len != 2){
      *out = (char*)(malloc(18));
      memcpy(*out, "DEL COMMAND ERROR", 18);
      return result;
    }
    dict_entry * entry = dictFind(client->db->dict, client->argv[1]);
    if(entry == NULL){
      result = NULL;
      *out = (char*)malloc(strlen("integer 0") + 1);
      memcpy(*out, "integer 0", strlen("integer 0") + 1);
      return result;
    }
    // 对对象的编码进行判断
  }
  return result;
}

