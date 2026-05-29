#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>

typedef struct  {
    int valid;
    uint32_t tag;
    int time;
    uint8_t *data;
} CacheLine;

typedef struct {
    CacheLine *lines;
} CacheSet;

typedef struct {
    CacheSet *sets;
    int S, s;
    int B, b;
    int E;
    int hits;
    int misses;
    int evictions;
    char name[8];
} Cache;

void initializeCache(Cache *cache, int s, int E, int b, const char *name);
void freeCache(Cache *cache);
int read_cache(Cache *cache, uint32_t addr, uint8_t *outBlock);
void place_cache(Cache *cache, uint32_t addr, uint8_t *block_in, int *time);
int store_cache(Cache *cache, uint32_t addr, int size, uint8_t *data);
#endif