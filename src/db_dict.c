#include "db_dict.h"

static unsigned long _dictNextPower(unsigned long size);
long _dictKeyIndex(dict *d, const void *key, uint64_t hash, dict_entry **existing);
int dictCompareKeys(const void * key,const void * _key);
int dictSetKey(dict_entry **existing, void *key);
int dictSetVal(dict_entry **existing,  void * val);
void * dictGetVal(dict_entry * existing);
void dictFreeVal(dict *d,dict_entry * he);
void dictFreeKey(dict *d,dict_entry * he);
int _dictClear(dict *d, db_dict *ht, void(callback)(void *));
static void _dictRehashStep(dict *d);

uint64_t hash_function(dict *d, const void * key){
  d->privdata = NULL;
  unsigned int hash = 5381;
  const char * str = (char*)key;
  while (*str){  
    hash = ((hash << 5) + hash) + (*str++); /* times 33 */  
  }  
  hash &= ~(1 << 31); /* strip the highest bit */  
  return hash; 
}

void dict_reset(db_dict *ht){
  ht->dict_entry = NULL;
  ht->size = 0;
  ht->size_mask = 0;
  ht->used = 0;
}
int _dict_init(dict *ht, void *privaDataPtr){
  dict_reset(&ht->hash_table[0]);
  dict_reset(&ht->hash_table[1]);
  ht->privdata = privaDataPtr;
  ht->rehashindex = -1;
  ht->iterators = 0;
  return DICT_OK;
}
// 创建一个字典对象
dict *dict_Create(void * privDataPtr){
  dict * ht = (dict *)malloc(sizeof(*ht));
  _dict_init(ht, privDataPtr);
  return ht;
}
// 拓展或创建 hash table
int dictExpand(dict *d, unsigned long size){
  db_dict n;
  unsigned long realsize = _dictNextPower(size);
  if(dictIsRehashing(d) || d->hash_table[0].used > size){ 
    // 如果大小小于哈希表中已经存在的元素的数量，则该大小无效
    return DICT_ERR;
  }
  if(realsize == d->hash_table[0].used)
    return DICT_ERR;

  // 分配新的哈希表并将所有指针初始化为NULL
  n.size = realsize;
  n.size_mask = realsize - 1;
  n.dict_entry = (dict_entry**)calloc(1, realsize * sizeof(dict_entry*));
  n.used = 0;
  if(d->hash_table[0].dict_entry == NULL){
    // 第一次分配
    d->hash_table[0] = n;
    return DICT_OK;
  }
  // 准备做 rehash
  d->hash_table[1] = n;
  d->rehashindex = 0;
  return DICT_OK;
}
// 我们的哈希表能力是2的幂
static unsigned long _dictNextPower(unsigned long size){
  unsigned long i = DICT_INSTALL_SIZE;
  if(size >= LONG_MAX){
    return LONG_MAX + 1LU;
  }
  while(1){
    if(i >= size){
      return i;
    }
    i *= 2;
  }
}

// ********************
// ADD
// 向目标哈希表添加一个元素
int dictAdd(dict *d, void *key, void *val){
  dict_entry * entry = dictAddRaw(d, key, NULL);  
  if(! entry) return DICT_ERR;
  dictSetVal(&entry, val);
  return DICT_OK;
}
// 这个函数添加条目，但不是设置一个值
// 而是向用户返回dictEntry结构，这将确保按用户的意愿填充value字段。
dict_entry *dictAddRaw(dict *d, void *key, dict_entry **existing){
  long index;
  dict_entry * entry;
  db_dict *ht;

  if(dictIsRehashing(d)){ 
    _dictRehashStep(d);
  }

  uint64_t i = hash_function(d, key);
  if((index = _dictKeyIndex(d, key, i, existing)) == -1)
    return NULL;
  // 在顶部插入元素，假设在数据库系统中，最近添加的条目更容易被访问
  if(dictIsRehashing(d)){
    ht = &d->hash_table[1];
  }else
    ht = &d->hash_table[0];
  entry = (dict_entry *)malloc(sizeof(struct dict_entry));
  entry->_next = ht->dict_entry[index];
  ht->dict_entry[index] = entry;
  ++ht->used;
  dictSetKey(&entry, key);
  return entry;
}
// 空代表没找到
dict_entry * dictFind(dict *d, const void *key){
  dict_entry * he;
  uint64_t h,idx,table;
  // dict is empty
  if(d->hash_table[0].used + d->hash_table[1].used == 0) return NULL;

  if(dictIsRehashing(d)) _dictRehashStep(d);
  h = hash_function(d, key);  
  for(table = 0 ; table <= 1; ++table){
    idx = h % d->hash_table[table].size;
    he = d->hash_table[table].dict_entry[idx];
    while(he){
      if (key==he->_key || dictCompareKeys(key, he->_key))
        return he;
      he = he->_next;
    }
    if(!dictIsRehashing(d)) return NULL;
  }
  return NULL;
}
// 返回给定键的值
void *dictFetchValue(dict *d, const void *key){
  dict_entry * he;
  he = dictFind(d, key);
  return he ? dictGetVal(he) : NULL;
}

// 搜索并删除一个元素 助手函数
static dict_entry *dictGenericDelete(dict *d, const void *key, int nofree){
  uint64_t h, idx;
  dict_entry * he, * prevHe;
  int table;

  if(d->hash_table[0].used == 0 && d->hash_table[1].used == 0) return NULL;

  if(dictIsRehashing(d)) _dictRehashStep(d);
  h = hash_function(d, key);

  for(table = 0; table <= 1; ++table){
    idx = h % d->hash_table[table].size;
    he = d->hash_table[table].dict_entry[idx];
    prevHe= NULL;
    while(he){
      if(key == he->_key || dictCompareKeys(key, he->_key)){
        if(prevHe){
          prevHe->_next = he->_next;
        }else{
          d->hash_table[table].dict_entry[idx] = he->_next;
        }
        if(!nofree){
          dictFreeVal(d, he);
          dictFreeKey(d, he);
          free(he);
        } // 释放资源
      d->hash_table[table].used--;
      return he;
      }
      prevHe = he;
      he = he->_next;
    }
    if(!dictIsRehashing(d)) break;
  }
  return NULL;
} 

// 从字典中删除给定键所所对应的键值对
int dictDelete(dict *d, const void *key){
  return dictGenericDelete(d, key, 0) ? DICT_OK : DICT_ERR;
}
// 释放字典
void dictRelease(dict *d){
  _dictClear(d, &d->hash_table[0], NULL);
  _dictClear(d, &d->hash_table[1], NULL);
  free(d);
}

// 如果仍有键要从旧哈希表移动到新哈希表则返回 1 否则返回 0
// 每次搬移一个桶
int dictRehash(dict *d, int n){
  int empty_visit = n * 10; // 最多可参观的空桶数
  if(!dictIsRehashing(d)) return 0;
  while(n-- && d->hash_table[0].used > 0){
    dict_entry *de, *nextDe;
    assert(d->hash_table[0].size > (unsigned long)d->rehashindex);
    while(d->hash_table[0].dict_entry[d->rehashindex] == NULL){
      d->rehashindex++;
      if(--empty_visit == 0) return 1;
    }
    de = d->hash_table[0].dict_entry[d->rehashindex];
    while(de){
      uint64_t h;
      nextDe = de->_next;
      h = hash_function(d, de->_key) % d->hash_table[1].size;
      de->_next = d->hash_table[1].dict_entry[h];
      // ***********
      d->hash_table[1].dict_entry[h] = de;
      d->hash_table[0].used--;
      d->hash_table[1].used++;
      de = nextDe;
    }
    d->hash_table[0].dict_entry[d->rehashindex] = NULL;
    d->rehashindex++;
  }
  //  检查是否已经 rehash 所有空间
  if(d->hash_table[0].used == 0){
    free(d->hash_table[0].dict_entry);
    d->hash_table[0] = d->hash_table[1];
    dict_reset(&d->hash_table[1]);
    d->rehashindex = -1;
    return 0;
  }
  return 1;
}


//..........................................

// 如果需要，展开哈希表
static int _dictExpandIfNeeded(dict *d){
  // 拓展哈希已经在进行中
  if(dictIsRehashing(d)) return DICT_OK;
  // 如果哈希表是空的，则将其扩展到初始大小。
  if(d->hash_table[0].size == 0) return dictExpand(d, DICT_INSTALL_SIZE);
  // 
  if(d->hash_table[0].used >= d->hash_table[0].size &&
     d->hash_table[0].used/d->hash_table[0].size > 1 ){
    return dictExpand(d, d->hash_table[0].used * 2);
  }
  return DICT_OK;
} 
long _dictKeyIndex(dict *d, const void *key, uint64_t hash, dict_entry **existing){
  unsigned long idx, table;     
  dict_entry * he;
  if(existing)
    *existing = NULL;
  // 如果需要，展开哈希表
  if(_dictExpandIfNeeded(d) == DICT_ERR)
    return -1;
  for(table = 0 ; table <= 1 ; table++){
    idx = (hash % d->hash_table[table].size);
    he = d->hash_table[table].dict_entry[idx];
    while(he != NULL){
      if(key == he->_key || dictCompareKeys(key, he->_key)){
        if(existing) *existing = he;
        return -1;
      }
      he = he->_next;
    }
    if(!dictIsRehashing(d)) break;
  }
  return idx;
}

int dictCompareKeys(const void * key,const void * _key){
  char * p1 = (char*)key;
  char * p2 = (char*)_key;
  int ret = strcmp(p1, p2);
  if(ret == 0) return 1;
  return 0;
}


//
int dictSetKey(dict_entry **existing, void *key){
  int len = strlen((char*)key);
  (*existing)->_key = malloc(len + 1);
  if((*existing)->_key == NULL) return DICT_ERR;
  memcpy((*existing)->_key, key, len);
  ((char*)(*existing)->_key)[len] = '\0';
  return DICT_OK;
}
int dictSetVal(dict_entry **existing,  void * val){
  (*existing)->val = val;
  return DICT_OK;
}
void * dictGetVal(dict_entry * existing){
  return existing->val;
}

void dictFreeVal(dict *d,dict_entry * he){
  d->privdata = NULL;
  free(he->val);
}
void dictFreeKey(dict *d,dict_entry * he){
  d->privdata = NULL;
  free(he->_key);
}
//摧毁整个字典
int _dictClear(dict *d, db_dict *ht, void(callback)(void *)){
  unsigned long i;
  if(callback){}
  // 先释放所有元素
  for( i = 0; i < ht->size && ht->used > 0; ++i){
    dict_entry *he, *nextHe;
    if((he = ht->dict_entry[i]) == NULL) continue;
    while(he){
      nextHe = he->_next;
      dictFreeVal(d, he);
      dictFreeKey(d, he);
      free(he);
      ht->used--;
      he = nextHe;
    }
  }
  free(ht->dict_entry);
  dict_reset(ht);  
  return DICT_ERR;
}
static void _dictRehashStep(dict *d) {
    if (d->iterators == 0) dictRehash(d,1);
}
