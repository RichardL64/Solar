/*

    RegisterCache
    https://github.com/RichardL64

    Manage a central list of register address/data pairs to update from the inverter

    The list is populated by any client requesting a register, then data updated once for all clients periodically

    R.A.Lincoln       July 2022

*/


//  Return the cache index for the passed address - ready for get or set Register
//  If size is passed it is updated, otherwise defaults to 1
//
int registerIndex(int address, int size = -1) {
  unsigned long now = millis();

  //  Is it already in the list?
  //
  for(int i = 0; i < CACHE_SIZE; i++) {
    if(cacheAddress[i] != address) continue;     // Ignore until we find it    
    if(size != -1) cacheSize[i] = size;          // update size if passed
    return i;
  }
  
  // If I didn't find it, find the oldest entry
  //
  int found =0;
  unsigned long oldest = now;
  for(int i = 0; i<CACHE_SIZE; i++) {           // check all cache entries
    if(cacheAge[i] < oldest) {
      oldest = cacheAge[i];
      found = i;
    }
  }
  
  //  Store the address register entry
  //
  cacheAddress[found] = address;
  if(size == -1) size = 1;                      // if not passed, default to size=1
  cacheSize[found] = size;
  cacheData[found] = 0;
  cacheState[found] = DATA_NULL;
  cacheAge[found] = now;                        // always store new aging

  return found;                                 // return the registers's cache index
}

//  Return the current data value of a register
//  Update aging on requests
//
long getRegister(int address, int size){
  int i = registerIndex(address, size);
  cacheAge[i] = millis();                       // update age on all requests

  if(cacheState[i] != DATA_VALID) return 0;     // no data
  
  return cacheData[i];
}

//  Return the data vlaue of a register as a JSON string
//  Update aging on requests
//   ,"address":<data>" or ""
//
String getJSON(int address, int size) {
  int i = registerIndex(address, size);
  cacheAge[i] = millis();                       // update age on all requests
  
  if(cacheState[i] != DATA_VALID) return "";    // no data

  return ",\"" + String(address) + "\":" + String(cacheData[i]);
}

//  Set a register data value in cache from its address
//
void setRegister(int address, long data, int state) {
  int i = registerIndex(address);
  cacheData[i] = data;
  cacheState[i] = state;  
}

//  Set a register data value from its index
//
void setCache(int i, long data, int state) {
  cacheData[i] = data;
  cacheState[i] = state;
}

//  Stop collecting the passed address
//  If the register wasn't collecting this will enter it then remove
//
void stopRegister(int address) {
  int i = registerIndex(address);   // find it
  cacheDelete(i);                   // remove it
}

void stopAll() {
  for(int i = 0; i < CACHE_SIZE; i++) {
    cacheDelete(i);
  }
}

//  Check the cache entry age, if old - remove it
//  Implies it has not been accessed in CACHE_OLD millis
//
void cacheAgeCheck(int i) {
  if(cacheAge[i] + CACHE_OLD > millis()) return;    // still inside the age limit

  cacheDelete(i);                                   // old entry - remove it
}

//  Discard a cache entry completely
//
void cacheDelete(int i) {
  cacheAddress[i] = 0;                              // old entry - remove it
  cacheSize[i] = 0;
  cacheData[i] = 0;
  cacheState[i] = DATA_NULL;
  cacheAge[i] = 0;
}
