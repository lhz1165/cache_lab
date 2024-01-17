#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "cachelab.h"
#include "hashmap.h"

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

int hitsTime = 0;
int missesTime = 0;
int evictionsTime = 0;

int main(int argc, char *argv[]) {

    int s_value = 0;
    int b_value = 0;
    int e_value = 0;

    // 解析命令行参数
    int option;
    while ((option = getopt(argc, argv, "s:b:e:")) != -1) {
        switch (option) {
            case 's':
                s_value = atoi(optarg);
                break;
            case 'b':
                b_value = atoi(optarg);
                break;
            case 'e':
                e_value = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -v <value> -b <value>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    CACHE *cachePtr = initCache(s_value, e_value, b_value);


    //读文件操作
    char line[256];
    FILE *testFile = fopen("D:\\c\\cache-lab\\yi.txt", "r");
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
        for (int j = 0; j < 1ULL << e; ++j) {
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
    for (int i = 0; i < linNum; ++i) {
        //逐行遍历
        CACHE_SET_LINE *lineStr = setPtr->myLine[i];
        //找打tag hit
        if (lineStr->tag == tag) {
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
    if (emptyNum > 0) {
        for (int i = 0; i < linNum; ++i) {
            CACHE_SET_LINE *lineStr = setPtr->myLine[i];
            if (lineStr->tag == -1) {
                lineStr->tag = tag;
                lineStr->block = b;
                lRUCachePut(lruPtr, tag, b);
            }
        }
        missesTime++;
    }
    //满了
    if (fullNum == e) {
        missesTime++;
        evictionsTime++;
        lRUCachePut(lruPtr, tag, b);

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



