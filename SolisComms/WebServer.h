/*

    WebServer.h
    https://github.com/RichardL64

    R.A.Lincoln       July 2022

*/

#include "SolisDashboard.html.h"

void httpHeader(WiFiClient client, int refresh=0);
void httpPrint(WiFiClient client, const char *data, int l = -1);
