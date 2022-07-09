/*

    RegisterCache.h
    https://github.com/RichardL64
    
    Manage a central list of register address/data pairs to update from the inverter

    Client requests get pooled so the inverter load, responding to requests scales by the number of registers
    not the number of clients making requests.

    At 9600 bps it takes around 1 second per register value retrieval
    
    R.A.Lincoln       July 2022

*/

#define CACHE_OLD 1000*60*5                   // minutes entry is not requested, remove it
#define CACHE_SIZE 20                         // Register cache table across all clients

#define DATA_NULL 0                           // no record ever returned
#define DATA_VALID 1                          // data is good
#define DATA_ERROR 2                          // modbus/rs485 threw an error on retreival

int           cacheAddress[CACHE_SIZE];       // Register base address
int           cacheSize[CACHE_SIZE];          // 1 or 2 registers
long          cacheData[CACHE_SIZE];          // Register value - enuogh space for a double register
byte          cacheState[CACHE_SIZE];         // flag re condition of the data value
unsigned long cacheAge[CACHE_SIZE];           // millis last time this register was retreived


//  Prototypes

void setRegister(int address, long data, int state = DATA_VALID);
void setCache(int i, long data, int state = DATA_VALID);
