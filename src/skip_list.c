#include "skip_list.h"

void strfree(str s);
int zslRandomLevel(void);
int zslValueGteMin(double value, zrangespec *spec);
int zslValueLteMax(double value, zrangespec *spec);

// 创建具有指定级别数的 skiplist 节点
zskiplistNode *zslCreateNode(int level, double score, str ses){
  int size = sizeof(struct zskiplistLevel);
  zskiplistNode *zn = 
    (zskiplistNode *)malloc(sizeof(*zn)+level*sizeof(struct zskiplistLevel));
  zn->score = score;
  zn->ses = ses;
  return zn;
}
// 创建一个新的 skiplist
zskiplist *zslCreate(void) {
  int j;
  zskiplist *zsl;

  zsl = (zskiplist *)malloc(sizeof(*zsl));
  zsl->level = 1;
  zsl->length = 0;
  zsl->header = zslCreateNode(ZSKIPLIST_MAXLEVEL,0,NULL);
  for (j = 0; j < ZSKIPLIST_MAXLEVEL; j++) {
    zsl->header->level[j].forward = NULL;
    zsl->header->level[j].span = 0;
  }
  zsl->header->backward = NULL;
  zsl->tail = NULL;
  return zsl;
}
// 释放指定的skiplist节点。元素的引用 str 字符串表示也被释放
void zslFreeNode(zskiplistNode *node) {
  strfree(node->ses);
  free(node);
}

// 释放整个跳跃表
void zslFree(zskiplist *zsl) {
  zskiplistNode *node = zsl->header->level[0].forward, *next;
  free(zsl->header);
  while(node) {
    next = node->level[0].forward;
    zslFreeNode(node);
    node = next;
  }
  free(zsl);
}
// 将给定键值添加到跳跃表中
zskiplistNode *zslInsert(zskiplist *zsl, double score, str ses){
  zskiplistNode *update[ZSKIPLIST_MAXLEVEL], *x;
  unsigned int rank[ZSKIPLIST_MAXLEVEL];
  int i, level;

  x = zsl->header;
  for(i = zsl->level - 1; i >= 0; --i){
    // 存储为达到插入位置而交叉的等级
    rank[i] = i == (zsl->level - 1) ? 0 : rank[i - 1]; 
    while (x->level[i].forward &&
           (x->level[i].forward->score < score ||
            (x->level[i].forward->score == score &&
             strcmp(x->level[i].forward->ses,ses) != 0)))
    {
      rank[i] += x->level[i].span;
      x = x->level[i].forward;
    }
    update[i] = x;          
  }
  // 我们假设元素不在哈希表中，因为我们允许重复得分，所以不应该重新插入相同的元素
  level = zslRandomLevel();   
  if (level > zsl->level) {
    for (i = zsl->level; i < level; i++) {
      rank[i] = 0;
      update[i] = zsl->header;
      update[i]->level[i].span = zsl->length;
    }
    zsl->level = level;
  }

  x = zslCreateNode(level,score,ses);
  for (i = 0; i < level; i++) {
    x->level[i].forward = update[i]->level[i].forward;
    update[i]->level[i].forward = x;

    /* update span covered by update[i] as x is inserted here */
    x->level[i].span = update[i]->level[i].span - (rank[0] - rank[i]);
    update[i]->level[i].span = (rank[0] - rank[i]) + 1;
  }

  /* increment span for untouched levels */
  for (i = level; i < zsl->level; i++) {
    update[i]->level[i].span++;
  }
  x->backward = (update[0] == zsl->header) ? NULL : update[0];
  if (x->level[0].forward)
    x->level[0].forward->backward = x;
  else
    zsl->tail = x;
  zsl->length = zsl->length + 1;
  return x;
}
// 删除节点
void zslDeleteNode(zskiplist *zsl, zskiplistNode *x, zskiplistNode **update) {
  int i;
  for (i = 0; i < zsl->level; i++) {
    if (update[i]->level[i].forward == x) {
      update[i]->level[i].span += x->level[i].span - 1;
      update[i]->level[i].forward = x->level[i].forward;
    } else {
      update[i]->level[i].span -= 1;
    }
  }
  if (x->level[0].forward) {
    x->level[0].forward->backward = x->backward;
  } else {
    zsl->tail = x->backward;
  }
  while(zsl->level > 1 && zsl->header->level[zsl->level-1].forward == NULL)
    zsl->level--;
  zsl->length--;
}
// 删除跳跃表中包含给定成员和分值的节点
int zslDelete(zskiplist *zsl, double score, str ses, zskiplistNode **node) {
  zskiplistNode *update[ZSKIPLIST_MAXLEVEL], *x;
  int i;

  x = zsl->header;
  for (i = zsl->level-1; i >= 0; i--) {
    while (x->level[i].forward &&
           (x->level[i].forward->score < score ||
            (x->level[i].forward->score == score &&
             strcmp(x->level[i].forward->ses,ses) < 0)))
    {
      x = x->level[i].forward;
    }
    update[i] = x;
  }
  /* We may have multiple elements with the same score, what we need
   * is to find the element with both the right score and object. */
  x = x->level[0].forward;
  if (x && score == x->score && strcmp(x->ses,ses) == 0) {
    zslDeleteNode(zsl, x, update);
    if (!node)
      zslFreeNode(x);
    else
      *node = x;
    return 1;
  }
  return 0; /* not found */
}
// 返回给定成员在跳跃表中的排位
unsigned long zslGetRank(zskiplist *zsl, str ses) {
  zskiplistNode *x;
  unsigned long rank = 0;
  unsigned long i;
  x = zsl->header;
  x = x->level[0].forward;

  for(i = 0; i< zsl->length; i++){
    if(strcmp(ses, x->ses) != 0){
      rank++;
    }else{
      break;
    }
    x = x->level[0].forward;
  }


  /* x might be equal to zsl->header, so test if obj is non-NULL */
  if (x->ses && strcmp(x->ses,ses) == 0) {
    return rank + 1;
  }
  return 0;
}
// 返回在给定排位上的节点
zskiplistNode* zslGetElementByRank(zskiplist *zsl, unsigned long rank) {
  zskiplistNode *x;
  unsigned long traversed = 0;
  int i;

  x = zsl->header;
  for (i = zsl->level-1; i >= 0; i--) {
    while (x->level[i].forward && (traversed + x->level[i].span) <= rank)
    {
      traversed += x->level[i].span;
      x = x->level[i].forward;
    }
    if (traversed == rank) {
      return x;
    }
  }
  return NULL;
}
// 给定一个分值范围 判断是否在跳跃表的分值范围内 
int zslIsInRange(zskiplist *zsl, zrangespec *range) {
  zskiplistNode *x;

  /* Test for ranges that will always be empty. */
  if (range->min > range->max ||
      (range->min == range->max && (range->minex || range->maxex)))
    return 0;
  x = zsl->tail;
  if (x == NULL || !zslValueGteMin(x->score,range))
    return 0;
  x = zsl->header->level[0].forward;
  if (x == NULL || !zslValueLteMax(x->score,range))
    return 0;
  return 1;
}
// 
zskiplistNode *zslFirstInRange(zskiplist *zsl, zrangespec *range) {
  zskiplistNode *x;
  int i;
  /* If everything is out of range, return early. */
  if (!zslIsInRange(zsl,range)) return NULL;

  x = zsl->header;
  for (i = zsl->level-1; i >= 0; i--) {
    /* Go forward while *OUT* of range. */
    while (x->level[i].forward &&
           !zslValueGteMin(x->level[i].forward->score,range))
      x = x->level[i].forward;
  }

  /* This is an inner range, so the next node cannot be NULL. */
  x = x->level[0].forward;
  assert(x != NULL);

  /* Check if score <= max. */
  if (!zslValueLteMax(x->score,range)) return NULL;
  return x;
}
// 
zskiplistNode *zslLastInRange(zskiplist *zsl, zrangespec *range) {
  zskiplistNode *x;
  int i;

  /* If everything is out of range, return early. */
  if (!zslIsInRange(zsl,range)) return NULL;

  x = zsl->header;
  for (i = zsl->level-1; i >= 0; i--) {
    /* Go forward while *IN* range. */
    while (x->level[i].forward &&
           zslValueLteMax(x->level[i].forward->score,range))
      x = x->level[i].forward;
  }
  /* This is an inner range, so this node cannot be NULL. */
  assert(x != NULL);
  /* Check if score >= min. */
  if (!zslValueGteMin(x->score,range)) return NULL;
  return x;
}
//
unsigned long zslDeleteRangeByScore(zskiplist *zsl, zrangespec *range) {
  zskiplistNode *update[ZSKIPLIST_MAXLEVEL], *x;
  unsigned long removed = 0;
  int i;

  x = zsl->header;
  for (i = zsl->level-1; i >= 0; i--) {
    while (x->level[i].forward && (range->minex ?
                                   x->level[i].forward->score <= range->min :
                                   x->level[i].forward->score < range->min))
      x = x->level[i].forward;
    update[i] = x;
  }

  /* Current node is the last with score < or <= min. */
  x = x->level[0].forward;

  /* Delete nodes while in range. */
  while (x &&
         (range->maxex ? x->score < range->max : x->score <= range->max))
  {
    zskiplistNode *next = x->level[0].forward;
    zslDeleteNode(zsl,x,update);
    zslFreeNode(x); /* Here is where x->ele is actually released. */
    removed++;
    x = next;
  }
  return removed;
}
//
unsigned long zslDeleteRangeByRank(zskiplist *zsl, unsigned int start, unsigned int end) {
  zskiplistNode *update[ZSKIPLIST_MAXLEVEL], *x;
  unsigned long traversed = 0, removed = 0;
  int i;

  x = zsl->header;
  for (i = zsl->level-1; i >= 0; i--) {
    while (x->level[i].forward && (traversed + x->level[i].span) < start) {
      traversed += x->level[i].span;
      x = x->level[i].forward;
    }
    update[i] = x;
  }

  traversed++;
  x = x->level[0].forward;
  while (x && traversed <= end) {
    zskiplistNode *next = x->level[0].forward;
    zslDeleteNode(zsl,x,update);
    zslFreeNode(x);
    removed++;
    traversed++;
    x = next;
  }
  return removed;
}











// *********************************************

// 返回要创建的新skiplist节点的随机级别。
int zslRandomLevel(void) {
  int level = 1;
  while ((random()&0xFFFF) < (ZSKIPLIST_P * 0xFFFF))
    level += 1;
  return (level<ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
}

void strfree(str s) {
  if (s == NULL) return;
  free(s);
}
int zslValueGteMin(double value, zrangespec *spec) {
  return spec->minex ? (value > spec->min) : (value >= spec->min);
}

int zslValueLteMax(double value, zrangespec *spec) {
  return spec->maxex ? (value < spec->max) : (value <= spec->max);
}






