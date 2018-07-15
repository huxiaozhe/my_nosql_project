#pragma once
#include "string_object.h"
#include "db_dict.h"

#define ZSKIPLIST_MAXLEVEL 32
#define ZSKIPLIST_P 0.5

typedef struct zskiplistNode {
  str ses;
  double score; // 分值
  struct zskiplistNode *backward; // 后退指针
  struct zskiplistLevel { 
    struct zskiplistNode *forward; // 前进指针
    unsigned int span; // 跨度
  } level[];
} zskiplistNode;

typedef struct zskiplist {
    struct zskiplistNode *header, *tail;
    unsigned long length; // 跳表目前包含的节点的数量
    int level; // 目前跳表内,层数最大的节点 不包括头节点
} zskiplist;

typedef struct zset {
  dict *dict;
  zskiplist *zsl;
} zset;

typedef struct {
  double min, max;
  int minex, maxex; /* are min or max exclusive? */
} zrangespec;

zskiplistNode *zslCreateNode(int level, double score, str ses);
zskiplist *zslCreate(void);
void zslFreeNode(zskiplistNode *node);
void zslFree(zskiplist *zsl);
zskiplistNode *zslInsert(zskiplist *zsl, double score, str ses);
void zslDeleteNode(zskiplist *zsl, zskiplistNode *x, zskiplistNode **update);
int zslDelete(zskiplist *zsl, double score, str ses, zskiplistNode **node);
unsigned long zslGetRank(zskiplist *zsl, str ses);
zskiplistNode* zslGetElementByRank(zskiplist *zsl, unsigned long rank);
int zslIsInRange(zskiplist *zsl, zrangespec *range);
zskiplistNode *zslFirstInRange(zskiplist *zsl, zrangespec *range);
zskiplistNode *zslLastInRange(zskiplist *zsl, zrangespec *range);
unsigned long zslDeleteRangeByScore(zskiplist *zsl, zrangespec *range);
unsigned long zslDeleteRangeByRank(zskiplist *zsl, unsigned int start, unsigned int end);








