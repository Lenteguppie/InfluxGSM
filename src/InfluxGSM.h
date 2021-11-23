#ifndef _INFLUXGSM_H_
#define _INFLUXGSM_H_

#include <Arduino.h>

#include "DataPoint.h"
#include "ArduinoHttpClient.h"
#include "Options.h"
#include "ConnectionInfo.h"
#include "util/debug.h"
#include "util/helpers.h"

class InfluxGSM
{
public:
    InfluxGSM();
    InfluxGSM(const char *serverUrl, uint16_t port, Client &comClient, const char *org, const char *bucket, const char *authToken);

    void setConnectionInfo(const char *serverUrl, uint16_t port, Client &comClient, const char *org, const char *bucket, const char *authToken);
    void setPrecision(WritePrecision precision);

    String pointToLineProtocol(const Point& point);
    
    bool writePoint(DataPoint &dp);
    bool isConnected();

    bool init();
private:
    
    void constructEndPoints();

    int doPOST(String path, String data);
    
    ConnectionInfo connectionInfo;
    HttpClient _client;
    WritePrecision _precision;

    String _writePath;
    String _statusPath;
    
}
#endif