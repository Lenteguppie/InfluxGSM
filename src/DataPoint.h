#ifndef _POINT_H_
#define _POINT_H_

#include <Arduino.h>
#include "WritePrecision.h"
#include "util/helpers.h"

/**
 * Class DataPoint represents InfluxDB point in line protocol.
 * It defines data to be written to InfluxDB.
 */
class DataPoint {
friend class InfluxGSM;
  public:
    DataPoint(String measurement);
    // Adds string tag 
    void addTag(String name, String value);
    // Add field with various types
    void addField(String name, float value, int decimalPlaces = 2)         { if(!isnan(value)) putField(name, String(value, decimalPlaces)); }
    void addField(String name, double value, int decimalPlaces = 2)        { if(!isnan(value)) putField(name, String(value, decimalPlaces)); }
    void addField(String name, char value)          { addField(name, String(value).c_str()); }
    void addField(String name, unsigned char value) { putField(name, String(value)+"i"); }
    void addField(String name, int value)           { putField(name, String(value)+"i"); }
    void addField(String name, unsigned int value)  { putField(name, String(value)+"i"); }
    void addField(String name, long value)          { putField(name, String(value)+"i"); }
    void addField(String name, unsigned long value) { putField(name, String(value)+"i"); }
    void addField(String name, bool value)          { putField(name, bool2string(value)); }
    void addField(String name, String value)        { addField(name, value.c_str()); }
    void addField(String name, long long value);
    void addField(String name, unsigned long long value);
    void addField(String name, const char *value);
    // Set timestamp to `now()` and store it in specified precision, nanoseconds by default. Date and time must be already set. See `configTime` in the device API
    void setTime(WritePrecision writePrecision = WritePrecision::NS);
    // Set timestamp in offset since epoch (1.1.1970). Correct precision must be set InfluxDBClient::setWriteOptions.
    void setTime(unsigned long long timestamp);
    // Set timestamp in offset since epoch (1.1.1970 00:00:00). Correct precision must be set InfluxDBClient::setWriteOptions.
    void setTime(String timestamp);
    // Clear all fields. Usefull for reusing point  
    void clearFields();
    // Clear tags
    void clearTags();
    // True if a point contains at least one field. Points without a field cannot be written to db
    bool hasFields() const { return _fields.length() > 0; }
    // True if a point contains at least one tag
    bool hasTags() const   { return _tags.length() > 0; }
    // True if a point contains timestamp
    bool hasTime() const   { return _timestamp.length() > 0; }
    // Creates line protocol with optionally added tags
    String toLineProtocol() const;
    // returns current timestamp
    String getTime() const { return _timestamp; }

    bool write();

  protected:
    String _measurement;
    String _tags;
    String _fields;
    String _timestamp;
  protected:    
    // method for formating field into line protocol
    void putField(String name, String value);
    // Creates line protocol string
    String createLineProtocol() const;
};
#endif //_POINT_H_