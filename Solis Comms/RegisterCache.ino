/*

    RegisterCache
    https://github.com/RichardL64

    Manage a central list of register address/data pairs to update from the inverter

    The list is populated by any client requesting a register, then data updated once for all clients periodically

    R.A.Lincoln       July 2022

*/


//  Start colleting data for a register
//  Return the cache index where data is stored - ready for get or set Register
//  Size supports registers which span multiple addresses
//
int collectRegister(int address, int size=-1, bool updateAging=false) {
  unsigned long now = millis();

  //  Is it already in the list?
  //
  for(int i=0; i<CACHE_SIZE; i++) {
    if(cacheAddress[i] == address) {            // already in it
      if(size != -1) {                          // update size if passed
        cacheSize[i] = size;
      }
      if(updateAging) {
        cacheAge[i] = now;                       // update the aging on request
      }
      return i;
    }
  }

  // If I didn't find it, - put it in the oldest slot
  //
  int found =0;
  unsigned long oldest = now;
  for(int i = 0; i<CACHE_SIZE; i++) {           // check all cache entries
    if(cacheAge[i] < oldest) {
      oldest = cacheAge[i];
      found = i;
    }
  }

  cacheAddress[found] = address;                // store the register
  if(size == -1) {                              // if not passed, default to size=1
    size = 1;
  }
  cacheSize[found] = size;
  cacheData[found] = 0;
  cacheAge[found] = now;                        // always store new aging
  return found;                                 // return the registers's cache index
}

//  Return the current data value of a register
//  Will update the aging since the value is being used
//
unsigned long getRegister(int address, int size){
  return cacheData[collectRegister(address, size, true)];
}

//  Set a register data value in cache
//  Does not updating aging
//
void setRegister(int address, unsigned long data) {
  cacheData[collectRegister(address)] = data;
}

//  Stop collecting the passed address
//  If the register wasn't collecting this will enter it then remove
//
void stopRegister(int address) {
  int i = collectRegister(address); // find it
  cacheAddress[i] = 0;              // remove it
  cacheSize[i] = 0;
  cacheData[i] = 0;
  cacheAge[i] = 0;
}

void stopAll() {
  for(int i = 0; i < CACHE_SIZE; i++) {
    cacheAddress[i] = 0;
    cacheSize[i] = 0;
    cacheData[i] = 0;
    cacheAge[i] = 0;
  }
}

//  If the entry has an address and is younger than CACHE_OLD millis return true
//  Else - remove the entry and return false
//
bool isFresh(int i) {
  if(cacheAddress[i] != 0 && cacheAge[i] + CACHE_OLD > millis()) {
    return true;
  }

  Serial.print("isFresh ");
  Serial.print(i);
  Serial.print(" age");
  Serial.print(cacheAge[i]+CACHE_OLD);
  Serial.print("vs.");
  Serial.print(millis());
  Serial.println();

  cacheAddress[i] = 0;            // remove the old entry
  cacheSize[i] = 0;
  cacheData[i] = 0;
  cacheAge[i] = 0;
  return false;
}

