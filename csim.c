#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "cachelab.h"

typedef struct node{
    int val;
    int key;
    struct node *pre;
    struct node *next;
}Node,*LinkList;//双向链表节点结构

typedef struct {
    LinkList store;//用来存放数据
    LinkList *next;//使用拉链发处理冲突
}Hash;//哈希表数据结构

typedef struct {
    int size;//当前缓存大小
    int capacity;//缓存容量
    Hash* table;//哈希表
    LinkList head;// 指向最近使用的数据
    LinkList tail;// 指向最久未使用的数据
} LRUCache;

typedef struct {
    int flag;
    int tag;
    int block;
} CACHE_SET_LINE;

typedef struct {
    int flag;
    CACHE_SET_LINE *myLine[512];
    int linNum;
    LRUCache *lruCache;

} CACHE_SET;

typedef struct {
    CACHE_SET *mySet[512]; // 数组中每个元素是一个 set 结构体
    int setNum;
} CACHE;


CACHE *initCache(int s, int e, int b);

void handelM(CACHE *cachePtr, char *from, char *to, int s, int e, int b);

void handelL(CACHE *cachePtr, char *from, char *to, int s, int e, int b);

void handelS(CACHE *cachePtr, char *from, char *to, int s, int e, int b);

void printBinary(int num);

LRUCache* lRUCacheCreate(int capacity);

void lRUCachePut(LRUCache* obj, int key, int value,int *deleteKey);

void lRUCacheFree(LRUCache* obj);

int lRUCacheGet(LRUCache* obj, int key);

int hitsTime = 0;
int missesTime = 0;
int evictionsTime = 0;

int main(int argc, char *argv[]) {

    int s_value = 0;
    int b_value = 0;
    int e_value = 0;
    char t[1000];
    // 解析命令行参数
    char opt;
    while (-1 != (opt = getopt(argc, argv, "hvs:E:b:t:")))
    {
        switch (opt)
        {
            case 'h':
            case 'v':
                //verbose = 1;
                break;
            case 's':
                s_value = atoi(optarg);
                break;
            case 'E':
                e_value = atoi(optarg);
                break;
            case 'b':
                b_value = atoi(optarg);
                break;
            case 't':
                strcpy(t, optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -v <value> -b <value>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    CACHE *cachePtr = initCache(s_value, e_value, b_value);


    //读文件操作
    char line[256];
    FILE *testFile = fopen(t, "r");
    if (testFile == NULL) {//若打开文件失败则退出
        puts("不能打开文件！");
        return 0;
    }
    // 逐行读取文件
    while (fgets(line, sizeof(line), testFile) != NULL) {
        char ins[8];
        char fromTo[8];
        sscanf(line, "%s %s", ins, fromTo);
        char *from;
        char *to;
        char *token;
        token = strtok(fromTo, ",");

        // 提取第一个字符串
        from = token;

        // 提取第二个字符串
        token = strtok(NULL, ",");
        to = token;

        printf("%s %s %s\n", ins, from, to);
        if (ins[0] == 'L') {
            handelL(cachePtr, from, to, s_value, e_value, b_value);
        } else if (ins[0] == 'S') {
            handelS(cachePtr, from, to, s_value, e_value, b_value);
        } else if (ins[0] == 'M') {
            handelM(cachePtr, from, to, s_value, e_value, b_value);
        } else {
            printf("instruct error %s\n", ins);
        }
    }

    // 关闭文件
    fclose(testFile);


    printSummary(hitsTime, missesTime, evictionsTime);
    return 0;
}

CACHE *initCache(int s, int e, int b) {
    CACHE *myCache = (CACHE *) malloc(sizeof(CACHE));
    myCache->setNum = 0;
    CACHE_SET *myCacheSet[s];
    CACHE_SET_LINE *myLine[e];

    for (int i = 0; i < 1ULL << s; ++i) {
        CACHE_SET *cs = (CACHE_SET *) malloc(sizeof(CACHE_SET));
        cs->flag = -1;
        cs->linNum = 0;
        cs->lruCache = lRUCacheCreate(e);
        for (int j = 0; j < e; ++j) {
            CACHE_SET_LINE *csl = (CACHE_SET_LINE *) malloc(sizeof(CACHE_SET_LINE));
            csl->block = 1ULL << b;
            csl->flag = 0;
            csl->tag = -1;
            cs->myLine[j] = csl;
            cs->linNum = cs->linNum + 1;
        }
        myCache->mySet[i] = cs;
        myCache->setNum = myCache->setNum + 1;
    }
    return myCache;

}

/**
 * move 2次访存
 */
void handelM(CACHE *cachePtr, char *from, char *to, int s, int e, int b) {
    handelS(cachePtr, from, to, s, e, b);
    handelS(cachePtr, from, to, s, e, b);

}

/**
 * load 1次访存
 */
void handelL(CACHE *cachePtr, char *from, char *to, int s, int e, int b) {
    handelS(cachePtr, from, to, s, e, b);
}

/**
 * store 1次访存
 */
void handelS(CACHE *cachePtr, char *from, char *to, int s, int e, int b) {
    char *endptr;
    long fromInt = strtol(from, &endptr, 16);
    printBinary(fromInt);
    int blockIndex = (fromInt) & ((1 << b) - 1);
    int setIndex = (fromInt >> b) & ((1 << s) - 1);
    int tag = (fromInt >> (b + s)) & ((1 << (64 - b - s)) - 1);
    printf("tag:%d setIndex:%d  blockindex:%d \n", tag, setIndex, blockIndex);
    CACHE_SET *setPtr = cachePtr->mySet[setIndex];
    LRUCache *lruPtr = setPtr->lruCache;
    int linNum = setPtr->linNum;
    int fullNum = 0;
    int emptyNum = 0;
    int hitFlag = 0;
    for (int i = 0; i < linNum; ++i) {
        //逐行遍历
        CACHE_SET_LINE *lineStr = setPtr->myLine[i];
        //找打tag hit
        if (lineStr->tag == tag) {
            hitFlag = 1;
            hitsTime++;
            lRUCacheGet(lruPtr, tag);
            break;
            //有空位置 最后处理
        } else if (lineStr->tag == -1) {
            emptyNum++;
        } else {
            //满了
            fullNum++;
        }

    }
    if (emptyNum > 0 && hitFlag == 0) {
        for (int i = 0; i < linNum; ++i) {
            CACHE_SET_LINE *lineStr = setPtr->myLine[i];
            if (lineStr->tag == -1) {
                lineStr->tag = tag;
                lineStr->block = b;
                int deleteKey = 1;
                lRUCachePut(lruPtr, tag, b,&deleteKey);
                break;
            }
        }
        missesTime++;
    }
    //满了
    if (fullNum == e) {
        missesTime++;
        evictionsTime++;
        //lru淘汰
        int deleteKey = -1;
        lRUCachePut(lruPtr, tag, b, &deleteKey);
        //替换
        for (int i = 0; i < setPtr->linNum; ++i){
            //逐行遍历替换
            CACHE_SET_LINE *lineStr = setPtr->myLine[i];
            if (lineStr->tag == deleteKey) {
                lineStr->tag = tag;
                break;
            }
        }
    }
}

void printBinary(int num) {
    for (int i = sizeof(int) * 8 - 1; i >= 0; i--) {
        // 通过右移操作获取每个位的值
        int bit = (num >> i) & 1;
        printf("%d", bit);

        // 在每四位之间添加空格，可选
        if (i % 4 == 0) {
            printf(" ");
        }
    }
    printf("\n");
}
Hash * HashMap(Hash *table,int key,int capacity){
    int addr = key % capacity;
    return &table[addr];
}
void HeadInsertion(LinkList head, LinkList cur)
{//双链表头插法
    if (cur->pre == NULL && cur->next == NULL)
    {// cur 不在链表中
        cur->pre = head;
        cur->next = head->next;
        head->next->pre = cur;
        head->next = cur;
    } else
    {// cur 在链表中
        LinkList first = head->next;//链表的第一个数据结点
        if ( first != cur)
        {//cur 是否已在第一个
            cur->pre->next = cur->next;//改变前驱结点指向
            cur->next->pre = cur->pre;//改变后继结点指向
            cur->next = first;//插入到第一个结点位置
            cur->pre = head;
            head->next = cur;
            first->pre = cur;
        }
    }
}
LRUCache* lRUCacheCreate(int capacity) {
    LRUCache* obj = (LRUCache*)malloc(sizeof(LRUCache));
    obj->table = (Hash*)malloc(capacity * sizeof(Hash));
    memset(obj->table, 0, capacity * sizeof(Hash));
    obj->head = (LinkList)malloc(sizeof(Node));
    obj->tail = (LinkList)malloc(sizeof(Node));    //创建头、尾结点并初始化
    obj->head->pre = NULL;
    obj->head->next = obj->tail;
    obj->tail->pre = obj->head;
    obj->tail->next = NULL;
    //初始化缓存 大小 和 容量
    obj->size = 0;
    obj->capacity = capacity;
    return obj;
}
int lRUCacheGet(LRUCache* obj, int key) {
    Hash* addr = HashMap(obj->table, key, obj->capacity);//取得哈希地址
    addr = addr->next;//跳过头结点
    if (addr == NULL){
        return -1;
    }
    while ( addr->next != NULL && addr->store->key != key)
    {//寻找密钥是否存在
        addr = addr->next;
    }   if (addr->store->key == key)
    {//查找成功
        HeadInsertion(obj->head, addr->store);//更新至表头
        return addr->store->val;
    }
    return -1;
}
void lRUCachePut(LRUCache* obj, int key, int value,int *deleteKey) {
    Hash* addr = HashMap(obj->table, key, obj->capacity);//取得哈希地址
    if (lRUCacheGet(obj, key) == -1)
    {
        //没找到key，并且lru满了
        if (obj->size>= obj->capacity)
        {
            LinkList last = obj->tail->pre->pre;
            LinkList del = last->next;
            *deleteKey=del->key;
            last->next = obj->tail;
            obj->tail->pre = last;
            Hash *delt = HashMap(obj->table,del->key,obj->capacity);//找到要删除的地址
            Hash *help_delt = delt;
            delt = delt->next;
            while(delt->store->key != del->key)
            {
                help_delt = delt;//删除的前一个节点
                delt = delt->next;
            }
            help_delt->next = delt->next;
            delt->store = NULL;
            delt->next=NULL;
            free(delt);

            Hash * new_insert = (Hash*)malloc(sizeof(Hash));
            new_insert->next = addr->next;
            addr->next = new_insert;
            new_insert->store = del;
            del->key = key;
            del->val=value;
            HeadInsertion(obj->head,del);
        }
        else
        {//LPU未满
            Hash* new_node = (Hash *)malloc(sizeof(Hash));
            new_node->store = (LinkList)malloc(sizeof(Node));
            new_node->next = addr->next;
            addr->next = new_node;
            new_node->store->pre  = NULL;
            new_node->store->next = NULL;
            new_node->store->val  = value;
            new_node->store->key  = key;
            HeadInsertion(obj->head,new_node->store);
            (obj->size)++;
        }
    }
    else
    {
        //找到key，直接移动到前面
        obj->head->next->val = value;//替换数据值
    }
}
void lRUCacheFree(LRUCache* obj) {
    free(obj->table);
    free(obj->head);
    free(obj->tail);
    free(obj);
}


