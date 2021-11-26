//#include <Arduino.h>
/* #region Device configuration */
#define VERSION_NUMBER "0.3"
#define DEVICE_TOKEN "4ea2353a-fc4d-4463-b244-1279243b4396"
/* #endregion */

/* #region  GSM configuration */
// set GSM PIN, if any
#define GSM_PIN "0000"

// Your GPRS credentials, if any
const char apn[] = "live.vodafone.com"; //SET TO YOUR APN
const char gprsUser[] = "vodafone";
const char gprsPass[] = "vodafone";

// Range to attempt to autobaud
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

#define TINY_GSM_TEST_GPRS true
#define TINY_GSM_TEST_GPS true
#define TINY_GSM_POWERDOWN true
/* #endregion */

/* #region  InfluxDB configuration */
#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif
// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "http://80.57.193.219:8086"
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN "bsDSnbA0CV5t8t39_0L4JWNC94GmUffo5iM2JzTsFLLwQjbSMGhUpN2NA8-Nv5pgSyjhjq6IcVT6CpC4BpJBxQ=="
// InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG "Smaddle"
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET "Smaddle device"

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

/* #endregion */

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include "DataPoint.h"

#include <SPI.h>
#include <Ticker.h>

TinyGsmClient client(modem);
HttpClient influx(client, INFLUXDB_URL, INFLUXDB_PORT);

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif


// Data points
DataPoint device("device");
/* #endregion */

#define uS_TO_S_FACTOR 1000000ULL // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 60          // Time ESP32 will go to sleep (in seconds)

#define UART_BAUD 9600
#define PIN_DTR 25
#define PIN_TX 27
#define PIN_RX 26
#define PWR_PIN 4

#define LED_PIN 12

float lat, lon;

void constructEndpoints()
{
    //Construct writepath endpoint
    writePath = "/api/v2/write";
    writePath += "bucket=";
    writePath += INFLUXDB_BUCKET;
    writePath += "&org=";
    writePath += INFLUXDB_ORG;
    writePath += "&precision=ms";
}

bool sendPOST(String path, String data)
{

    influx.beginRequest();
    influx.post(writePath);
    influx.sendHeader("Content-Type", "application/json");
    influx.sendHeader("Content-Length", data.length());
    influx.sendHeader("Authorization", String("TOKEN " + String(INFLUXDB_TOKEN)));
    influx.beginBody();
    influx.print(data);
    influx.endRequest();

    // read the status code and body of the response
    int statusCode = influx.responseStatusCode();
    String response = influx.responseBody();

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);

    return statusCode == 204;
}

void sendSensorData()
{
    device.clearFields();

    device.addField("lat", 1.3423);
    device.addField("lon", 3.434);
    device.addField("millis", millis());
    if (!sendPOST(writePath, device.toLineProtocol()))
    {
        Serial.println("Something went wrong sending data to InfluxDB");
    }
}

void enableGPS(void)
{
    // Set SIM7000G GPIO4 LOW ,turn on GPS power
    // CMD:AT+SGPIO=0,4,1,1
    // Only in version 20200415 is there a function to control GPS power
    modem.sendAT("+SGPIO=0,4,1,1");
    if (modem.waitResponse(10000L) != 1)
    {
        DBG(" SGPIO=0,4,1,1 false ");
    }
    modem.enableGPS();
}

void disableGPS(void)
{
    // Set SIM7000G GPIO4 LOW ,turn off GPS power
    // CMD:AT+SGPIO=0,4,1,0
    // Only in version 20200415 is there a function to control GPS power
    modem.sendAT("+SGPIO=0,4,1,0");
    if (modem.waitResponse(10000L) != 1)
    {
        DBG(" SGPIO=0,4,1,0 false ");
    }
    modem.disableGPS();
}

void modemPowerOn()
{
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, LOW);
    delay(1000); //Datasheet Ton mintues = 1S
    digitalWrite(PWR_PIN, HIGH);
}

void modemPowerOff()
{
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, LOW);
    delay(1500); //Datasheet Ton mintues = 1.2S
    digitalWrite(PWR_PIN, HIGH);
}

void modemRestart()
{
    modemPowerOff();
    delay(1000);
    modemPowerOn();
}

void gsmInit(void)
{
    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
    // Set GSM module baud rate
    // TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
    // SerialAT.begin(9600);

    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    Serial.println("Initializing modem...");
    if (!modem.init())
    {
        Serial.println("Failed to restart modem, attempting to continue without restarting");
    }

    String name = modem.getModemName();
    delay(500);
    Serial.println("Modem Name: " + name);

    String modemInfo = modem.getModemInfo();
    delay(500);
    Serial.println("Modem Info: " + modemInfo);

    // Set SIM7000G GPIO4 LOW ,turn off GPS power
    // CMD:AT+SGPIO=0,4,1,0
    // Only in version 20200415 is there a function to control GPS power
    modem.sendAT("+SGPIO=0,4,1,0");

#if TINY_GSM_USE_GPRS
    // Unlock your SIM card with a PIN if needed
    if (GSM_PIN && modem.getSimStatus() != 3)
    {
        modem.simUnlock(GSM_PIN);
    }
#endif
    /*
    2 Automatic
    13 GSM only
    38 LTE only
    51 GSM and LTE only
  * * * */
    String res;
    do
    {
        res = modem.setNetworkMode(2);
        delay(500);
    } while (res != "1");

    /*
    1 CAT-M
    2 NB-Iot
    3 CAT-M and NB-IoT
  * * */
    do
    {
        res = modem.setPreferredMode(1);
        delay(500);
    } while (res != "1");
}

void connectLTE()
{
#if TINY_GSM_USE_GPRS && defined TINY_GSM_MODEM_XBEE
    // The XBee must run the gprsConnect function BEFORE waiting for network!
    modem.gprsConnect(apn, gprsUser, gprsPass);
#endif

    SerialMon.print("Waiting for network...");
    if (!modem.waitForNetwork())
    {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
    SerialMon.println(" success");

    if (modem.isNetworkConnected())
    {
        SerialMon.println("Network connected");
    }

    SerialMon.print(F("Connecting to "));
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass))
    {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
    SerialMon.println(" success");

    if (modem.isGprsConnected())
    {
        SerialMon.println("GPRS connected");
    }
}

void disconnectLTE()
{
#if TINY_GSM_USE_WIFI
    modem.networkDisconnect();
    SerialMon.println(F("WiFi disconnected"));
#endif
#if TINY_GSM_USE_GPRS
    modem.gprsDisconnect();
    SerialMon.println(F("GPRS disconnected"));
#endif
}

void setup()
{
    // Set console baud rate
    SerialMon.begin(9600);

    delay(10);

    // Set LED OFF
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    delay(300);
    digitalWrite(PWR_PIN, LOW);

    modemPowerOn();

    gsmInit();

    client.setInsecure(true);

    device.addTag("token", DEVICE_TOKEN);
    device.addTag("version", VERSION_NUMBER);

    Serial.println("/**********************************************************/");
    Serial.println("To initialize the network test, please make sure your GPS");
    Serial.println("antenna has been connected to the GPS port on the board.");
    Serial.println("/**********************************************************/\n\n");

    delay(10000);
}

void loop()
{
    if (!modem.testAT())
    {
        Serial.println("Failed to restart modem, attempting to continue without restarting");
        modemRestart();
        return;
    }

    Serial.println("Start positioning . Make sure to locate outdoors.");
    Serial.println("The blue indicator light flashes to indicate positioning.");

    enableGPS();

    while (1)
    {
        if (modem.getGPS(&lat, &lon))
        {
            Serial.println("The location has been locked, the latitude and longitude are:");
            Serial.print("latitude:");
            Serial.println(lat);
            Serial.print("longitude:");
            Serial.println(lon);
            break;
        }
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        delay(2000);
    }

    disableGPS();

    Serial.println(F("/**********************************************************/"));
    Serial.println(F("Sending the data to InfluxDB."));
    Serial.println(F("/**********************************************************/\n\n"));

    connectLTE();
    sendSensorData();
    disconnectLTE();

    delay(60000);
}