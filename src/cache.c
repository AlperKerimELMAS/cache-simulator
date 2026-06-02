#include <stdlib.h>
#include <string.h>
#include "../include/cache.h"

void initializeCache(Cache *cache, int s, int E, int b, const char *name)
{
    cache->s = s;
    cache->E = E;
    cache->b = b;
    cache->S = 1 << s;
    cache->B = 1 << b;
    cache->hits = 0;
    cache->misses = 0;
    cache->evictions = 0;
    strncpy(cache->name, name, 7);
    cache->name[7] = '\0';
    cache->sets = malloc(cache->S * sizeof(CacheSet));

    int numberOfSets = cache->S;
    int blockSize = cache->B;
    int i, j;
    for (i = 0; i < numberOfSets; i++)
    {
        cache->sets[i].lines = malloc(E * sizeof(CacheLine));
        for (j = 0; j < E; j++)
        {
            CacheLine *temp = &cache->sets[i].lines[j];
            temp->valid = 0;
            temp->data = malloc(blockSize);
            memset(temp->data, 0, blockSize);
        }
    }
}

void freeCache(Cache *cache)
{
    int i, j;
    int numberOfSets = cache->S;
    int numberOfLines = cache->E;

    for (i = 0; i < numberOfSets; i++)
    {
        for (j = 0; j < numberOfLines; j++)
        {
            free(cache->sets[i].lines[j].data);
        }
        free(cache->sets[i].lines);
    }
    free(cache->sets);
}

int read_cache(Cache *cache, uint32_t addr, uint8_t *outBlock)
{
    uint32_t tag = addr >> (cache->b + cache->s);
    uint32_t index = (addr >> cache->b) & ((1 << cache->s) - 1);
    int i;
    for (i = 0; i < cache->E; i++)
    {
        if (cache->sets[index].lines[i].valid && cache->sets[index].lines[i].tag == tag)
        {
            cache->hits++;

            if (outBlock != NULL)
                memcpy(outBlock, cache->sets[index].lines[i].data, cache->B);
            return 1;
        }
    }
    cache->misses++;
    return 0;
}

void place_cache(Cache *cache, uint32_t addr, uint8_t *block_in, int *time)
{
    uint32_t tag = addr >> (cache->b + cache->s);
    uint32_t index = (addr >> cache->b) & ((1 << cache->s) - 1);

    int target = -1;
    int associativity = cache->E;
    int i;
    for (i = 0; i < associativity; i++)
    {
        if (!cache->sets[index].lines[i].valid)
        {
            target = i;
            break;
        }
    }
    // Eviction (FIFO)
    if (target == -1)
    {
        cache->evictions++;
        target = 0;
        int minTime = cache->sets[index].lines[0].time;
        for (i = 0; i < associativity; i++)
        {
            if (cache->sets[index].lines[i].time < minTime)
            {
                target = i;
                minTime = cache->sets[index].lines[i].time;
            }
        }
    }

    cache->sets[index].lines[target].valid = 1;
    cache->sets[index].lines[target].tag = tag;
    (*time)++;
    cache->sets[index].lines[target].time = *time;
    memcpy(cache->sets[index].lines[target].data, block_in, cache->B);
}

int store_cache(Cache *cache, uint32_t addr, int size, uint8_t *data)
{
    uint32_t tag = addr >> (cache->b + cache->s);
    uint32_t index = (addr >> cache->b) & ((1 << cache->s) - 1);
    uint32_t offset = addr & ((1 << cache->b) - 1);

    int associativity = cache->E;
    int i;

    for (i = 0; i < associativity; i++)
    {
        if (cache->sets[index].lines[i].valid && cache->sets[index].lines[i].tag == tag)
        {
            cache->hits++;
            memcpy(cache->sets[index].lines[i].data + offset, data, size);
            return 1;
        }
    }

    cache->misses++;
    return 0;
}