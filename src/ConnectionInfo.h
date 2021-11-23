#ifndef _CONNECTIONINFO_H_
#define _CONNECTIONINFO_H_

struct ConnectionInfo
{
    // Connection info
    String serverUrl;
    uint16_t port;
    Client &comClient;
    // Write & query targets
    String bucket;
    String org;
    // v2 authetication token
    String authToken;
    // Version of InfluxDB 1 or 2
    uint8_t dbVersion;
    // V1 user authetication
    String user;
    String password;
    // Certificate info
    const char *certInfo;
    // flag if https should ignore cert validation
    bool insecure;
    // Error message of last failed operation
    String lastError;
};
#endif