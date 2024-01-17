//
// Created by 21907 on 2024/1/17.
//

#ifndef CACHE_LAB_HASHMAP_H
#define CACHE_LAB_HASHMAP_H

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
LRUCache* lRUCacheCreate(int capacity);

void lRUCachePut(LRUCache* obj, int key, int value);

void lRUCacheFree(LRUCache* obj);

int lRUCacheGet(LRUCache* obj, int key);

#endif //CACHE_LAB_HASHMAP_H