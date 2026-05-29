#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cache.h"
#include <stdint.h>

#define RAM_SIZE (16 * 1024 * 1024)

uint8_t *ram;
int globalTime = 0;
Cache L1I, L1D, L2;
uint8_t outBlockL2[256];

// hex to bytes, 2 hex char  = 1 byte logic
void hexToBytes(const char *hexStr, uint8_t *byteArray, int size) {
    int i;
    for(i = 0; i < size; i++)
        sscanf(hexStr + 2 * i, "%2hhx", &byteArray[i]);
}

void process_load(char operation, uint32_t addr) {
    Cache *l1 = (operation == 'I') ? &L1I : &L1D;

    int l1Hit = read_cache(l1, addr, NULL);
    int l2Hit = read_cache(&L2, addr, outBlockL2);

    printf(" %s %s, L2 %s \n", l1->name, l1Hit ? "hit" : "miss", l2Hit ? "hit" : "miss");

    if(!l1Hit && !l2Hit) { // not in the cache
        uint32_t offset = addr & ((1 << l1->b) - 1);
        uint32_t blockStart = addr - offset;
        uint8_t block[256];
        memcpy(block, &ram[blockStart], l1->B);

        place_cache(&L2, blockStart, block, &globalTime);
        place_cache(l1, blockStart, block, &globalTime);
    
        uint32_t l2Index = (addr >> L2.b) & ((1 << L2.s) - 1);
        printf(" Place in L2 set %d, %s\n", l2Index, l1->name);
    }
    else if(!l1Hit && l2Hit) { // Not in l1 cache but already in l2 cache
        uint32_t offset = addr & ((1 << l1->b) - 1);
        uint32_t blockStart = addr - offset;

        place_cache(l1, blockStart, outBlockL2, &globalTime);
        printf(" Place in %s\n", l1->name);
    }
}

void process_store(uint32_t addr, int size, uint8_t *data) {
    int l1Hit = store_cache(&L1D, addr, size, data);
    int l2Hit = store_cache(&L2, addr, size, data);

    printf(" L1D %s, L2 %s\n", l1Hit ? "hit" : "miss", l2Hit ? "hit" : "miss");
    memcpy(&ram[addr], data, size);
    printf(" Store in ");
    if(l1Hit)
        printf("L1D ");
    if(l2Hit)
        printf("L2 ");
    printf("RAM\n");
    
}

int main(int argc, char *argv[]) {
    
    int L1s=0, L1E=0, L1b=0, L2s=0, L2E=0, L2b=0;
    char trace_file[40] = {'\0'};

    for (int i = 1; i < argc; i++) { // argv[0] is the name of the program
        if (strcmp(argv[i], "-L1s") == 0)
            L1s = atoi(argv[++i]);
        else if (strcmp(argv[i], "-L1E") == 0) 
            L1E = atoi(argv[++i]);
        else if (strcmp(argv[i], "-L1b") == 0) 
            L1b = atoi(argv[++i]);
        else if (strcmp(argv[i], "-L2s") == 0) 
            L2s = atoi(argv[++i]);
        else if (strcmp(argv[i], "-L2E") == 0) 
            L2E = atoi(argv[++i]);
        else if (strcmp(argv[i], "-L2b") == 0) 
            L2b = atoi(argv[++i]);
        else if (strcmp(argv[i], "-t") == 0) 
            strcpy(trace_file, argv[++i]);
    }

    ram = calloc(RAM_SIZE, 1);
    FILE *file = fopen("../data/RAM.dat", "rb"); // read as binary

    if (file) {
        fread(ram, 1, RAM_SIZE, file);
        fclose(file);
    } else 
        printf("RAM.dat could not be found. No memory load performed!\n");
        
    initializeCache(&L1I, L1s, L1E, L1b, "L1I");
    initializeCache(&L1D, L1s, L1E, L1b, "L1D");
    initializeCache(&L2, L2s, L2E, L2b, "L2");

    char traceLocation[60];
    sprintf(traceLocation, "../data/traces/%s", trace_file);
    FILE *trace = fopen(traceLocation, "r");
    if (!trace) {
        printf("%s does not exist.\n", trace_file);
        return 1;
    }

    char line[200];
    while(fgets(line, 200, trace)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) < 2)
            continue;
        
        printf("%s\n", line);
        char operation;
        uint32_t address;
        int size;
        char dataString[64];
        uint8_t dataBytes[32];

        if(line[0] == 'L' || line[0] == 'I') {
            sscanf(line, "%c %x, %d", &operation, &address, &size);
            process_load(operation, address);
        }
        else if(line[0] == 'S') {
            sscanf(line, "%c %x, %d, %s", &operation, &address, &size, dataString);
            hexToBytes(dataString, dataBytes, size);
            process_store(address, size, dataBytes);
        }
        else if (line[0] == 'M') {
            sscanf(line, "%c %x, %d, %s", &operation, &address, &size, dataString);
            hexToBytes(dataString, dataBytes, size);
            process_load('L', address);
            process_store(address, size, dataBytes);
        }
    }
    fclose(trace);
    printf("\n");
    printf("L1I-hits:%d L1I-misses:%d L1I-evictions:%d\n", L1I.hits, L1I.misses, L1I.evictions);
    printf("L1D-hits:%d L1D-misses:%d L1D-evictions:%d\n", L1D.hits, L1D.misses, L1D.evictions);
    printf("L2-hits:%d L2-misses:%d L2-evictions:%d\n", L2.hits, L2.misses, L2.evictions);

    freeCache(&L1I);
    freeCache(&L1D);
    freeCache(&L2);
    free(ram);

    return 0;
}