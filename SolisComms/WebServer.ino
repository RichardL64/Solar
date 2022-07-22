/*

    WebServer
    https://github.com/RichardL64

    Functions relating to web client interaction
    Interpreting and responding to HTTP requests

    No validation of inbound formats here, intended for running on a private/local network

    R.A.Lincoln       July 2022

*/

#define HTTP_PRINT_CHUNK 1024
#define CHAR_WAIT 100

//  Check for a wifi client and service inbound HTTP requests
//  Generally reads the first header line to create a response
//  Other HTTP header instructions are ignored
//
void serviceWiFi() {

  WiFiClient client = server.available();   // listen for incoming clients
  if (!client) return;

  Serial.print(F("new client "));
  Serial.print(client.remoteIP());
  Serial.print(":");
  Serial.println(client.remotePort());

  char line1[512];                                // line buffers
  char lineN[512];
   
  nextLine(client, line1);                        // keep the first line "... GET etc...."
  Serial.print(" ");
  Serial.println(line1);
  
  nextLine(client, lineN);
  while(lineN[0] != '\0') {                        // read the whole request - useful for debugging
    Serial.print(" ");
    Serial.println(lineN);
    nextLine(client, lineN);
  }

  parseLine(client, line1);                       // functionality all from line1 
  client.stop();                                  // disconnect
  
  Serial.println(F("client disconnected"));
}

//  Get the next line from the client
//  No checks for available characters, just try reading them and check for -1
//
void nextLine(WiFiClient client, char *line) {
    line[0] = '\0';
    int pos = 0;

    while (client.connected()) {            // loop while the client is connected

      unsigned long cWait = millis();       // character timer
      char c = client.read();               // try and get a character
      switch (c) {
        case 0xff:                          // -1 = no characters - ignore pending the timeout
          if(millis() - cWait > CHAR_WAIT) 
            return;                         // ==> took too long
          delay(1);
          break;
          
        case '\r':                          // carriage return - ignored
          break;
          
        case '\n':                          // new line - end of record 'cr, lf' pair
          return;                           // ==> done
          break;
          
        default:                            // otherwise add the char to the line
          line[pos++] = c;
          line[pos] = '\0';
          break;
      }

    }    
}

//  Parse an inbound line from the client
//  Creates any activity and HTML response required
//
void parseLine(WiFiClient client, char *line) {
  char name[50], value[50], json[512];
      
  //  Historic entry points - useful for basic testing
  //
  if(strstr(line, "GET /H") != 0){              // LED on
    digitalWrite(LED_BUILTIN, HIGH);
    httpHeader(client);
    client.print(F("LED on"));
    httpFooter(client);
    return;
  }

  if(strstr(line,"GET /L") != 0){             // LED off
    digitalWrite(LED_BUILTIN, LOW);
    httpHeader(client);
    client.print(F("LED off"));
    httpFooter(client);
    return;
  }

  //  The dashboard HTML is modified to insert the actual server IP address for callbacks.
  //  We search for "<script>" and then "*** LIVE IP ADDRESS ***" then " to find the hostname
  //  The HTML is read only so replacement is done dnymaically on writing
  //
  if(strstr(line, "GET / ") != 0
    || strstr(line, "GET /dashboard") != 0) {                     // /dashboard?url=<value>
    httpHeader(client);

    char *url = strstr(dashboardHtml, "<script>");                // Script part of the HTML
    url = strstr(url, "*** LIVE IP ADDRESS ***");
    url = strstr(url, "\"") +1;                                   // leading "
    char *quote = strstr(url, "\"");                              // following "

    httpPrint(client, dashboardHtml, url - dashboardHtml);        // Print until the URL
    client.print(WiFi.localIP());                                 // Insert the live IP address
    httpPrint(client, quote);                                     // From the training quote to the end
        
    httpFooter(client);
    return;
  }

/*
  //  Unfinished - load/store password in local flash
  //
  if(line.startsWith("GET /AP")) {            // Access point config            /AP?SSID=<value>&PASSWORD=<value>
    int pos = 0;    
    String ssid, password;

    if(nextName(line, pos) == "ssid") {
      ssid = nextValue(line, pos);
    }
    if(nextName(line, pos) == "password") {
      password = nextValue(line, pos);
    }

    // save ssid/password to flash here
    httpHeader(client);
    client.println(F("SSID, Password set - reset to retry Wifi connection"));
    httpFooter(client);
    return;
  }
*/

  if(strstr(line, "GET /R") != 0) {           // Return register values           /R?refresh=<seconds>&address=<address>,<address>...
    char *pos = nextName(line, name);
    
    int refresh = 0;                          // ?refresh=n  parameter must come first for the header
    if(strcmp(name, "refresh") == 0) {
      pos = nextValue(pos, value);      
      refresh = atoi(value);
      pos = nextName(pos, name);
    }
    httpHeader(client, refresh);
        
    if(strcmp(name, "address") == 0) {        // ?address=<value>,<value>...
      pos = parseAddressValues(pos, json);
      client.print(json);
    }
    httpFooter(client);
    return;
  }

  if(strstr(line, "GET /S") != 0) {           // Stop collecting register values  /S?address=<address>
    char *pos = nextName(line, name);
    
    if(strcmp(name, "all") == 0) {                        // /S?all
      stopAll();

    } else if(strcmp(name, "address") == 0) {             // /S?address=<address>
      pos = nextValue(pos, value);
      stopRegister(atoi(value));

    }
    httpHeader(client);
    httpFooter(client);
    return;
  }

  if(strstr(line, "GET /C") != 0) {          // Readout the cache
    httpHeader(client);
    client.print(F("Register cache<br>"));
    for(int i = 0; i < CACHE_SIZE; i++) {
      client.print("[");
      client.print(i);
      client.print("] (");
      client.print(regCache[i].state);
      client.print(") @");
      client.print(regCache[i].age);
      client.print(" ");
      client.print(regCache[i].address);
      client.print(".");
      client.print(regCache[i].size);
      client.print(" = ");
      client.print(regCache[i].value);
      client.print("<br>");
    }
    httpFooter(client);
    return;
  }

  //  Fallthrough - send a confirmation header/footer anyway so the caller knows I'm here
  //
  httpHeader(client);
  httpFooter(client);
  
}

//  Send the standard HTTP header
//
void httpHeader(WiFiClient client, int refresh) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");    
    client.println("Access-Control-Allow-Origin: *");     // CORS allow cross site retreival
    if(refresh != 0) {                                    // Optional automatic refresh
      client.print("Refresh: ");
      client.println(refresh);
    }
    client.println();
}

//  Standard close out a request with an empty line
//
void httpFooter(WiFiClient client) {
    client.println();
}

//  Breakup large files into <4k chunks for output
//  WiFi.Client write/print over around 4k trashes data
//  If the length is passed its used, otherwise writes to end of string \0
//
void httpPrint(WiFiClient client, const char *data, int length) {
    if(length == -1) length = strlen(data);
    const char* p = data;
    for(; length > HTTP_PRINT_CHUNK; length -= HTTP_PRINT_CHUNK) {
      client.write(p, HTTP_PRINT_CHUNK);      // whole chunk
      p += HTTP_PRINT_CHUNK;
    }
    client.write(p, length);                  // partial left over
}

//  Next name "..... ?<name>=...&<name> ...."
//  name is updated with the name
//  returns a pointer to the next char
//
char *nextName(char *line, char *name) {
  return nextStr(line, "?&", "= ", name);    
}


//  Next value "..... =<value>,<value>&<parm>=... "
//  value is updated with the value
//  returns a pointer to the next char
//
char *nextValue(char *line, char *value) {
  return nextStr(line, "=,", ",&? ", value);  
}

//  Find the next string delimited by d1, d2 in line
//  Value is the found value
//  Returns a pointer to the char after the value
//  
char *nextStr(char *line, const char *d1, const char *d2, char *value) {
  value[0] = '\0';
  char *start = strpbrk(line, d1);
  char *end = strpbrk(start +1, d2);

  if(!start || !end) return line;             // no delimiter pair

  int l = end - start -1;
  strncpy(value, start +1, l);                // copy it into the value parm
  value[l] = '\0';                            // strncpy doesnt append \0
  
//  Serial.print("nextStr: ");
//  Serial.println(value);

  return end;
}


//  Addresses values are added to the lookup cache,
//  and returned as JSON JS array with their values
//  Returns the pointer after the last one
//
char *parseAddressValues(char *line, char *json) {
  char value[50];                                       

  //  Loop around each parameter value for a JSON address/value output
  //
  json[0] = '\0';
  char *pos = nextValue(line, value);         // First value
  while(value[0] != '\0') {                   // process all values on the line

    int address = atoi(value);
    int size = 1;                             // interpret optional <address>.1 or .2    
    if(strstr(value, ".2")) size = 2;

    getJSON(address, size, json);             // stores in the cache & appends ,"address":data
    pos = nextValue(pos, value);              // Next value
  }

  //  Top & tail the resulting JSON string
  //
  if(json[0] == '\0') {
    strcpy(json, "{}");                       // valid null json
  } else {
    json[0] = '{';                            // over the leading comma
    strcat(json, "}");                        // after the last value
  }

  return pos;
}
