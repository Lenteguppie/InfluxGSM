#include "DataPoint.h"


#define VERSION_NUMBER "0.3"
#define DEVICE_TOKEN "4ea2353a-fc4d-4463-b244-1279243b4396"

// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "http://80.57.193.219"
// InfluxDB server port, default 8086
#define INFLUXDB_PORT 8086
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN "bsDSnbA0CV5t8t39_0L4JWNC94GmUffo5iM2JzTsFLLwQjbSMGhUpN2NA8-Nv5pgSyjhjq6IcVT6CpC4BpJBxQ=="
// InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG "Smaddle"
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET "Smaddle device"

DataPoint sensor("sensor");

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  sensor.addTag("token", DEVICE_TOKEN);
  sensor.addTag("version", VERSION_NUMBER);
}

void loop() {
  // put your main code here, to run repeatedly:

  device.clearFields();

  sensor.addField("lat", 0.21);
  sensor.addField("lon", "test");
  sensor.addField("millis", millis());
  
  Serial.println(device.toLineProtocol());

  delay(5000);

}