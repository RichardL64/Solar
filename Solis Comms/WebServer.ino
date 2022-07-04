/*

    WebServer

    Functions relating to web client interaction
    Interpreting and responding to HTTP requests

    No validation of inbound formats here, intended for running on a private/local network

    R.A.Lincoln       July 2022

*/

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
//  Call client.stop() to finalise the client conversation
//
void parseLine(WiFiClient client, const String &line) {
  Serial.print(">");
  Serial.print(line);
  Serial.println("<");

  if(line.indexOf("GET /H") != -1){             // LED on
    digitalWrite(LED_BUILTIN, HIGH);
  }

  if(line.indexOf("GET /L") != -1){             // LED off
    digitalWrite(LED_BUILTIN, LOW);
  }

  if(line.indexOf("GET /R") != -1) {            // Return register values           /R?address=<address>,<address>...
    httpHeader(client);
    client.print(parseAddressParms(line));
    httpFooter(client);
  }

  if(line.indexOf("GET /S") != -1) {            // Stop collecting register values  /S?address=<address>
    int pos = 0;
    String name = nextName(line, pos);
    if(name.equalsIgnoreCase("all")) {                         // /S?all=
      stopAll();

    } else if(name.equalsIgnoreCase("address")) {              // /S?address=<address>
      stopRegister(nextValue(line, pos).toInt());

    }
  }

  if(line.indexOf("GET /C") != -1) {            // Readout the cache
    httpHeader(client);
    client.print("Register cache<br>");
    for(int i = 0; i<CACHE_SIZE; i++) {
      client.print(i);
      client.print(", ");
      client.print(cacheAddress[i]);
      client.print(", ");
      client.print(cacheSize[i]);
      client.print(", ");
      client.print(cacheData[i]);
      client.print(", ");
      client.print(cacheAge[i]);
      client.print("<br>");
    }
    httpFooter(client);
  }

  if(line.indexOf("GET /") != -1) {             // Default home page
    httpHeader(client);
    client.print("Hello World<br>");
    httpFooter(client);
  }
}

//  Send the standard HTTP header
//
void httpHeader(WiFiClient client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();
}

//  Standard close out a request with an empty line
//
void httpFooter(WiFiClient client) {
    client.println();
    client.stop();
}

//  Return the first index of any of the search string chars in line string after start
//
int firstOf(const String &line, const char* search, int start) {
  int len = line.length();
  for(int i = start; i < len; i++) {        // each char of the line
      const char* c = search;
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
//  line = "..... ?<name>=...&<name>= ...."
//
String nextName(const String &line, int &pos) {
  int mark = firstOf(line, "?&", pos);        // next ? or &
  int eq = firstOf(line, "=", mark);          // and next = after it
  
  if(mark != -1 && eq != -1) {                // found both delimiters
    pos = eq +1;
    return line.substring(mark +1, eq);
  }
  return String();                            // couldn't find both delimiters
}

//  Starting at pos, find the next value until , next parameter name or space
//  Return the value and increment pos to the first character after it
//
//  line = "..... <value>,<value>&<parm>=... "
//
String nextValue(const String &line, int &pos) {
  int start = pos;
  int end = firstOf(line, ",& ", pos);        // next value or parameter delimiter

  if(end != -1) {                             // if I found it
    pos = end +1;
    return line.substring(start, end);
  }

  return String();                            // couldn't find a delimiter
}

//  Parse address parameters in the string
//  Ignores any other paramaters present
//  ... ?address=value,value ...
//
//  Addresses are added to the lookup cache
//  and returned as JSON JS array with their values
//
String parseAddressParms(const String &line) {
  int pos = 0;

  if(!nextName(line, pos).equalsIgnoreCase("address")) {      // if no address parameters return empty JSON array
    return "{}";              
  }

  String json;
  json.reserve(512);

  String value = nextValue(line, pos);                        // First value
  while(value != "") {
    int size = 1;                                             // interpret optional <address>.<size>
    if(value.indexOf('.') != -1) {
      size = value.substring(value.indexOf('.')+1).toInt();
    }

    int address = value.toInt();
    unsigned long data = getRegister(address, size);          // get the latest value, and/or queue for retreival
    json += ",\"" + String(address) + "\":" + String(data);   // generates...  ,"address":data,"address":data

    value = nextValue(line, pos);                             // Next value
  }

  if(json.length() == 0) {                                    // complete formatting of the json string - top and tail with { }
    json = "{}";
  } else {
    json[0] = '{';
    json += '}';
  }

  return json;
}

