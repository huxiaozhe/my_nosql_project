#include "define.h"
#include "link_list.h"
#include "string_object.h"
#include "serialization.h"
#include "deserialization.h"
#include "nosql_object.h"
#include "skip_list.h"

int main( void ){
  char a1[] = "wang";
  char a2[] = "zhang";
  char a3[] = "li";
  zskiplist * list =  zslCreate();
  zslInsert(list, 1.0000, a1);
  zslInsert(list, 3.0000, a2);
  zslInsert(list, 2.0000, a3);

  zrangespec * range = (zrangespec *)malloc(sizeof(*range));
  range->min = 1.00000000;
  range->max = 3.00000000;
  zskiplistNode * node = zslFirstInRange(list, range);
  printf("node -> %f %s\n", node->score, node->ses);
  int count = 0;
  while(node != NULL && node->score <= range->max){
    count++;
    node = node->level[0].forward;
  }
  printf("%d\n", count);
  return 0;
}
