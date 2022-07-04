/*

    RegisterCache.h
    Manage a central list of register address/data pairs to update from the inverter

    Client requests get pooled so the inverter load, responding to requests scales by the number of registers
    not the number of clients making requests.

    R.A.Lincoln       July 2022

*/

#define CACHE_OLD (unsigned long)1000*60*10   // 10 minutes inactivity on an entry, remove it
#define CACHE_SIZE 20                         // Register cache table across all clients

int           cacheAddress[CACHE_SIZE];       // Register base address
int           cacheSize[CACHE_SIZE];          // 1 or 2 registers
unsigned long cacheData[CACHE_SIZE];          // enuogh space for a double register
unsigned long cacheAge[CACHE_SIZE];           // millis last time this was touched
