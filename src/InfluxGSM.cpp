#include "InfluxGSM.h"

static String precisionToString(WritePrecision precision, uint8_t version = 2)
{
    switch (precision)
    {
    case WritePrecision::US:
        return version == 1 ? "u" : "us";
    case WritePrecision::MS:
        return "ms";
    case WritePrecision::NS:
        return "ns";
    case WritePrecision::S:
        return "s";
    default:
        return "";
    }
}

InfluxGSM::InfluxGSM(const char *serverUrl, uint16_t port, Client &comClient, const char *org, const char *bucket, const char *authToken, WritePrecision precision)
{
    setConnectionInfo(serverUrl, port, comClient, org, bucket, authToken);
    setPrecision(precision);
}

void setPrecision(WritePrecision precision)
{
    _precision = precision;
}

void InfluxGSM::setConnectionInfo(const char *serverUrl, uint16_t port, Client &comClient, const char *org, const char *bucket, const char *authToken)
{
    connectionInfo.serverUrl = serverUrl;
    connectionInfo.port = port;
    connectionInfo.comClient = comClient;
    connectionInfo.org = org;
    connectionInfo.bucket = bucket;
    connectionInfo.authToken = authToken;

    _client = new HttpClient(connectionInfo.comClient, connectionInfo.serverUrl, connectionInfo.port);
}

String InfluxGSM::pointToLineProtocol(const Point& point) {
    return point.createLineProtocol(_writeOptions._defaultTags);
}

bool InfluxGSM::init()
{
    INFLUXDB_CLIENT_DEBUG("[D]  Initializing InfluxGSM a new instance");
    constructEndPoints();
    if (_connInfo.serverUrl.length() == 0 || (_connInfo.dbVersion == 2 && (_connInfo.org.length() == 0 || _connInfo.bucket.length() == 0 || _connInfo.authToken.length() == 0)))
    {
        INFLUXDB_CLIENT_DEBUG("[E] Invalid parameters\n");
        _connInfo.lastError = F("Invalid parameters");
        return false;
    }
    if (_connInfo.serverUrl.endsWith("/"))
    {
        _connInfo.serverUrl = _connInfo.serverUrl.substring(0, _connInfo.serverUrl.length() - 1);
    }
    if (!_connInfo.serverUrl.startsWith("http"))
    {
        _connInfo.lastError = F("Invalid URL scheme");
        return false;
    }

    return true;
}

void InfluxGSM::constructEndPoints()
{
    // construct write endpoint
    _writePath = "/write?";
    _writePath += "bucket=" + String(bucket);
    _writePath += "org=" + String(org);
    _writePath += "precision=" + precisionToString(_precision);
    INFLUXDB_CLIENT_DEBUG("[D]  Write path: %s\n", _writePath.c_str());

    _statusPath = "/health";
    INFLUXDB_CLIENT_DEBUG("[D]  Status path: %s\n", _statusPath.c_str());
}

bool InfluxGSM::writePoint(DataPoint &dp){
    String postData = pointToLineProtocol(dp);
    response = doPOST(_writePath, postData); 
    return (response == "204");
}

int InfluxGSM::doPOST(String path, String data)
{
    _client.beginRequest();
    _client.post(path.c_str());
    _client.sendHeader("Content-Type", "application/json");
    _client.sendHeader("Content-Length", data.length());
    _client.sendHeader("Authorization", String("TOKEN " + connectionInfo.authToken));
    _client.beginBody();
    _client.print(data);
    _client.endRequest();
    // read the status code and body of the response
    int statusCode = _client.responseStatusCode();
    String response = _client.responseBody();

    INFLUXDB_CLIENT_DEBUG("[D]  Response code: %s\n", statusCode.c_str());
    INFLUXDB_CLIENT_DEBUG("[D]  Response: %s\n", toString(response).c_str());
    return statusCode;
}