/*

    RegisterCache.h
    https://github.com/RichardL64
    
    Manage a central list of register address/data pairs to update from the inverter

    Client requests get pooled so the inverter load, responding to requests scales by the number of registers
    not the number of clients making requests.

    At 9600 bps it takes around 1 second per register value retrieval
    
    R.A.Lincoln       July 2022

*/

#define CACHE_OLD 1000*60*2                   // if a register is not requested for a while remove it
#define CACHE_SIZE 20                         // register cache table across all clients

#define STATE_NULL  0                         // no record ever returned
#define STATE_VALID 1                         // data is good
#define STATE_ERROR 2                         // modbus/rs485 threw an error on retreival

//  Local register cache
struct {
    byte state;                               // entry status STATE_
    unsigned long age;                        // millis last time this register was requested
    int address;                              // Register base address
    int size;                                 // 1 or 2 registers
    long value;                               // Register value - enuogh space for a double register
} regCache[CACHE_SIZE];

//  Prototypes
void setRegister(int address, long data, int state = STATE_VALID);
void setCache(int i, long data, int state = STATE_VALID);
