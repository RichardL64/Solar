/*

    WebServer
    https://github.com/RichardL64

    Functions relating to web client interaction
    Interpreting and responding to HTTP requests

    No validation of inbound formats here, intended for running on a private/local network

    R.A.Lincoln       July 2022

*/

#define HTTP_PRINT_CHUNK 1024


//  Check for a wifi client and service inbound HTTP requests
//  Generally reads the first header line to create a response
//  Other HTTP header instructions are ignored
//
void serviceWiFi() {

  WiFiClient client = server.available();   // listen for incoming clients
  if (!client) return;

  Serial.println("new client");
  
  String line = nextLine(client);
  parseLine(client, line);                // drive from the first line of each request
  client.stop();                          // disconnect
  
  Serial.println("client disconnected");
}

//  Get the next line from the client
//  Until end of line or out of characters or the client disconnects
//
String nextLine(WiFiClient client) {
    String line = "";
    line.reserve(512);

    while (client.connected()) {            // loop while the client is connected

      if (client.available()) {             // if there are bytes to read from the client
        char c = client.read();             // Process one character at a time
        switch (c) {
        case '\n':                          // new line
        case '\r':                          // carriage return
          return line;
          break;
        default:                            // add chars to the line
          line += c;
          break;
        }
      }
    }

    return line;
}

//  Parse an inbound line from the client
//  Creates any activity and HTML response required
//
void parseLine(WiFiClient client, const String &line) {
  Serial.println(line);

  if(line.startsWith("GET /H")){              // LED on
    digitalWrite(LED_BUILTIN, HIGH);
    httpHeader(client);
    client.print("LED on");
    httpFooter(client);
  }

  if(line.startsWith("GET /L")){              // LED off
    digitalWrite(LED_BUILTIN, LOW);
    httpHeader(client);
    client.print("LED off");
    httpFooter(client);
  }

  //  The dashboard HTML is modified to insert the actual server IP address for callbacks.
  //  We search for "<script>" and then "*** LIVE IP ADDRESS ***" then " to find the hostname
  //  The HTML is read only so replacement is done dnymaically on writing
  //
  if(line.startsWith("GET /dashboard")
    ||line.startsWith("GET / ")) {                                // /dashboard?url=<value>
    httpHeader(client);

    char *url = strstr(dashboardHtml, "<script>");                // Script part of the HTML
    url = strstr(url, "*** LIVE IP ADDRESS ***");
    url = strstr(url, "\"") +1;                                   // leading "
    char *quote = strstr(url, "\"");                              // following "

    httpPrint(client, dashboardHtml, url - dashboardHtml);        // Print until the URL
    client.print(WiFi.localIP());                                 // Insert the live IP address
    httpPrint(client, quote);                                     // From the training quote to the end
        
    httpFooter(client);
  }


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
    client.println("SSID, Password set - reset to retry Wifi connection");
    httpFooter(client);
  }

  if(line.startsWith("GET /R")) {            // Return register values           /R?refresh=<seconds>&address=<address>,<address>...
    int pos = 0;
    String name;
    
    name = nextName(line, pos);
    
    int refresh = 0;                         // ?refresh=n  parameter must come first for the header
    if(name == "refresh") {
      refresh = nextValue(line, pos).toInt();
      name = nextName(line, pos);
    }
    httpHeader(client, refresh);
    
    if(name == "address") {                  // ?address=<value>,<value>...
      client.print(parseAddressValues(line, pos));
    }
    httpFooter(client);
  }

  if(line.startsWith("GET /S")) {            // Stop collecting register values  /S?address=<address>
    int pos = 0;
    String name = nextName(line, pos);
    if(name.equalsIgnoreCase("all")) {                         // /S?all
      stopAll();

    } else if(name.equalsIgnoreCase("address")) {              // /S?address=<address>
      stopRegister(nextValue(line, pos).toInt());

    }
    httpHeader(client);
    httpFooter(client);
  }

  if(line.startsWith("GET /C")) {            // Readout the cache
    httpHeader(client);
    client.print("Register cache<br>");
    for(int i = 0; i<CACHE_SIZE; i++) {
      client.print("[");
      client.print(i);
      client.print("] ");
      client.print(cacheAddress[i]);
      client.print(".");
      client.print(cacheSize[i]);
      client.print(" = ");
      client.print(cacheData[i]);
      client.print(" (");
      client.print(cacheState[i]);
      client.print(") @");
      client.print(cacheAge[i]);
      client.print("<br>");
    }
    httpFooter(client);
  }

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

//  WiFi.Client write/print over around 4k trashes data
//  Breakup large files into <4k chunks for output
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


//  Return the first index of any of the search string chars in line string after start
//
int firstOf(const String &line, const char *search, int start) {
  int len = line.length();
  for(int i = start; i < len; i++) {        // each char of the line
      const char *c = search;
      while(*c != '\0') {                   // vs. each char of the search
          if(line[i] == *c){
              return i;                     // found one - stop now
          }
          c++;
      }
  }
  return -1;
}

//  Starting at pos, find the next parameter name, following ? or &
//  Return the parameter name and increment pos to the first character after it
//
//  line = "..... ?<name>=...&<name> ...."
//
String nextName(const String &line, int &pos) {
  int start = firstOf(line, "?&", pos);       // next ? or &
  int end = firstOf(line, "= ", start +1);    // and next = or space after it
  
  if(start == -1 
    || end == -1) return String();            // didn't find delimiters

  pos = end;                                  // found a name
  return line.substring(start +1, end);
}

//  Starting at pos, find the next value until , next parameter name or space
//  Return the value and increment pos to the first character after it
//
//  line = "..... =<value>,<value>&<parm>=... "
//
String nextValue(const String &line, int &pos) {
  int start = firstOf(line, "=,", pos);       // following a name or another value
  int end = firstOf(line, ",&? ", start +1);  // next value or parameter delimiter

  if(start ==-1 
    || end == -1) return String();            // no delimiter

  pos = end;                                  // found a value
  return line.substring(start +1, end);
}

//  Addresses values are added to the lookup cache,
//  and returned as JSON JS array with their values
//  Pos incremented after the last one.
//
String parseAddressValues(const String &line, int &pos) {
  String json;
  json.reserve(512);

  //  Loop around each parameter value for a JSON address/value output
  //
  String value = nextValue(line, pos);                        // First value
  while(value != "") {
    int size = 1;                                             // interpret optional <address>.<size>
    if(value.endsWith(".2")) size = 2;                        // nothing, .1 or .2

    int address = value.toInt();
    json += getJSON(address, size);                           // stores in the cache & generates...  ,\n"address":data

    value = nextValue(line, pos);                             // Next value
  }

  //  Top & tail the resulting JSON string
  //
  if(json.length() == 0) {                                    // complete formatting of the json string - top and tail with { }
    json = "{}";
  } else {
    json[0] = '{';
    json += "}";
  }

  return json;
}
