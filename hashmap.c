#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include "hashmap.h"

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
