/**
 * 
 * Point.cpp: Point for write into InfluxDB server
 * 
 * MIT License
 * 
 * Copyright (c) 2020 InfluxData
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include "DataPoint.h"
#include "util/helpers.h"

DataPoint::DataPoint(String measurement) : _measurement(escapeKey(measurement, false)),
                                           _tags(""),
                                           _fields(""),
                                           _timestamp("")
{
}

void DataPoint::addTag(String name, String value)
{
    if (_tags.length() > 0)
    {
        _tags += ',';
    }
    _tags += escapeKey(name);
    _tags += '=';
    _tags += escapeKey(value);
}

void DataPoint::addField(String name, long long value)
{
    char buff[50];
    snprintf(buff, 50, "%lld", value);
    putField(name, String(buff) + "i");
}

void DataPoint::addField(String name, unsigned long long value)
{
    char buff[50];
    snprintf(buff, 50, "%llu", value);
    putField(name, String(buff) + "i");
}

void DataPoint::addField(String name, const char *value)
{
    putField(name, "\"" + escapeValue(value) + "\"");
}

void DataPoint::putField(String name, String value)
{
    if (_fields.length() > 0)
    {
        _fields += ',';
    }
    _fields += escapeKey(name);
    _fields += '=';
    _fields += value;
}

String DataPoint::toLineProtocol() const
{
        String line;
    line.reserve(_measurement.length() + 1 + _tags.length() + 1 + _fields.length() + 1 + _timestamp.length());
    line += _measurement;
    if (hasTags())
    {
        line += ",";
        line += _tags;
    }
    if (hasFields())
    {
        line += " ";
        line += _fields;
    }
    if (hasTime())
    {
        line += " ";
        line += _timestamp;
    }
    return line;
}

void DataPoint::setTime(WritePrecision precision)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    switch (precision)
    {
    case WritePrecision::NS:
        setTime(getTimeStamp(&tv, 9));
        break;
    case WritePrecision::US:
        setTime(getTimeStamp(&tv, 6));
        break;
    case WritePrecision::MS:
        setTime(getTimeStamp(&tv, 3));
        break;
    case WritePrecision::S:
        setTime(getTimeStamp(&tv, 0));
        break;
    case WritePrecision::NoTime:
        _timestamp = "";
        break;
    }
}

void DataPoint::setTime(unsigned long long timestamp)
{
    _timestamp = timeStampToString(timestamp);
}

void DataPoint::setTime(String timestamp)
{
    _timestamp = timestamp;
}

void DataPoint::clearFields()
{
    _fields = (char *)nullptr;
    _timestamp = (char *)nullptr;
}

void DataPoint::clearTags()
{
    _tags = (char *)nullptr;
}