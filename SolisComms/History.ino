/*

    History
    https://github.com/RichardL64
    
    Manage collection of historic data
    Initially to Arduino RAM in cyclical buffers
    Potential to expand to external SD
        
    R.A.Lincoln       July 2022

*/

#define HISTORY_CACHE_SIZE 20

#define FIVE_MINUTE 0;
#define ONE_HOUR 1;
#define ONE_DAY 2;

//  History retention instructions
//
struct {
  int address;
  bool fiveMinute;
  bool oneHour;
  bool oneDay;
} histCache[HISTORY_CACHE_SIZE];


//  Record a address/value pair
//
void setHistory(int address, long value) {

  for(int i = 0; i< HISTORY_CACHE_SIZE; i++) {               // exists already?
    if(histCache[i].address == address) {

      //  Maintain a moving average
      //
/*      
      //  Check for writing out a data point
      //
      if(histCache[i].fiveMinute
      && its 5 minutes) {
        
      }
      if(histCache[i].oneHour
      && its on the hour) {
        
      }
      if(histCache[i].oneDay
      && its midnight?) {
        
      }
*/
      break;
    }
  }
}

//  New instruction to keep history records
//  Store in a file for future reference and globally for collection now
//
void keepHistory(char range, int freq, int keep, int address) {

  for(int i = 0; i < HISTORY_CACHE_SIZE; i++) {           // exists already?
    if(histCache[i].address == address) {
      if(range == 'm') histCache[i].fiveMinute = true;    
      if(range == 'h') histCache[i].oneHour = true;    
      if(range == 'd') histCache[i].oneDay = true;    
      saveHistCache;                                    // save to file
      return;
    }
  }

  for(int i = 0; i < HISTORY_CACHE_SIZE; i++) {          // add to an empty slot
    if(histCache[i].address == 0) {
      histCache[i].address = address;
      if(range == 'm') histCache[i].fiveMinute = true;    
      if(range == 'h') histCache[i].oneHour = true;    
      if(range == 'd') histCache[i].oneDay = true;    
      saveHistCache;                                    // save to file
      return;
    }
  }
}

//  Save the history instruction information to file
//
void saveHistCache() {
  
}

//  Load the history instruction information from file
//
void loadHistCache() {
  
}
