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
int read_cache(Cache *cache, unsigned addr);
void place_cache(Cache *cache, unsigned addr, unsigned *block_in, int *time);
int store_cache(Cache *cache, unsigned addr, int size, unsigned *data);
#endif