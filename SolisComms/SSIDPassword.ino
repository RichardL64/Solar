/*

    SSIDPassword
    https://github.com/RichardL64

    Manage non volatile storage and retrieval of SSID password
    
    R.A.Lincoln       July 2022

*/

#define FILE_NAME "SSIDPASSWORD"

void setSSIDPassword(String &SSID, String &Password) {

  WiFiStorageFile file = WiFiStorage.open(FILE_NAME);
  file.erase();
    
  String s = SSID + "," + Password;
  file.write(s.c_str(), s.length());

}

bool getSSIDPassword(String &SSID, String &Password) {

  WiFiStorageFile file = WiFiStorage.open(FILE_NAME);
  if(!file) return false;

  char buffer[128];
  file.seek(0);
  file.read(buffer, 128);
          
  String s = buffer;
  int comma = s.indexOf(',');
  SSID = s.substring(0,comma);
  Password = s.substring(comma +1);
  
  return true;
}
