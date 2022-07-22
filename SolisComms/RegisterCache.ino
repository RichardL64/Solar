/*

    RegisterCache
    https://github.com/RichardL64

    Manage a central list of register address/value pairs to update from the inverter

    The list is populated by any client requesting a register, then values updated once for all clients periodically

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
    if(regCache[i].address != address) continue;  // Ignore until we find it    
    if(size != -1) regCache[i].size = size;       // update size if passed
    return i;
  }

  // If I didn't find it, find the oldest entry
  //
  int found = 0;
  unsigned long oldest = now;

  for(int i = 0; i<CACHE_SIZE; i++) {             // check all cache entries
    if(regCache[i].age < oldest) {
      oldest = regCache[i].age;
      found = i;
    }
  }

  //  Store the address register entry
  //
  if(size == -1) size = 1;                        // if not passed, default to size=1
  regCache[found] = {STATE_NULL, now, address, size, 0};

  return found;                                   // return the registers's cache index
}

//  Return the current value of a register
//  Update aging on all value requests
//
long getRegister(int address, int size){
  int i = registerIndex(address, size);
  regCache[i].age = millis();                     // update age on all requests
  if(regCache[i].state != STATE_VALID) return 0;  // no data

  return regCache[i].value;
}

//  Append the value of a register to the json string
//  Update aging on all value requests
//   ,"address":<value>" or ""
//
void getJSON(int address, int size, char *json) {
  int i = registerIndex(address, size);           // find the cache entry
  regCache[i].age = millis();                     // update age on all requests
  if(regCache[i].state != STATE_VALID) return;    // no data

  char addressS[20], valueS[20];                  // format as strings
  itoa(address, addressS, 10);
  itoa(regCache[i].value, valueS, 10);

  strcat(json, ",\"");                            // construct the json entry
  strcat(json, addressS);
  strcat(json, "\":");
  strcat(json, valueS);
}

//  Set a register value in cache from its address
//
void setRegister(int address, long value, int state) {
  int i = registerIndex(address);
  setCache(i, value, state);
}

//  Set a register value from its index
//
void setCache(int i, long value, int state) {
  regCache[i].value = value;
  regCache[i].state = state;
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
  if(millis() - regCache[i].age < CACHE_OLD) return;  // still inside the age limit
  cacheDelete(i);                                     // old entry - remove it
}

//  Discard a cache entry completely
//
void cacheDelete(int i) {
  regCache[i] = {STATE_NULL, 0, 0, 0, 0};
}
