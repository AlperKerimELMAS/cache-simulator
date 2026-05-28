#ifndef CACHE_H
#define CACHE_H

typedef struct  {
    int valid;
    int tag;
    int time;
    int *data;
} CacheLine;

typedef struct {
    CacheLine *lines;
} CacheSet;

typedef struct {
    CacheSet *sets;
    int numberOfSets;
    int associativity;
    int blockSize;
    int hits;
    int misses;
    int evictions;
    int memoryReads;
    int memoryWrites;
} Cache;

Cache *initializeCache(int numberOfSets, int blockSize, int assocciativity);
void freeCache(Cache *cache);
void accessCache(Cache *cache, char instructionType, int address, int size, int *time);

#endif