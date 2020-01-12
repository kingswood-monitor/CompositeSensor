#pragma once

/**
 * Sensor interface for a Kingswood Monitor System sensor.
 * 
 * Creates a composite sensor from a collection of physical temperature, pressure, etc. sensors.
 * If a measurement is available from more than one type of device, it implements priotisation logic.
 * 
 * Usage:
 * 
 *      CompositeSensor mySensor;
 *      mySensor.begin();
 * 
 *      float temp = mySensor.readTemperature();
 */
#include <math.h>
#include "SparkFun_SCD30_Arduino_Library.h"
#include "Adafruit_VEML7700.h"
#include "DFRobot_BMP388_I2C.h"
#include "ClosedCube_HDC1080.h"

#define VBATPIN A9 // for measuring battery voltage

SCD30 scd30;
Adafruit_VEML7700 veml7700 = Adafruit_VEML7700();
DFRobot_BMP388_I2C bmp388;
ClosedCube_HDC1080 hdc1080;

double Round(double value, int places);

class CompositeSensor
{
public:
    struct __attribute__((packed)) SensorReadings
    {
        double temp;
        double humidity;
        int16_t co2;
        double light;
        int16_t pressure;
        double battery;
    };

    void begin();
    SensorReadings readSensors();

    bool hasSCD30 = false;
    bool hasBMP388 = false;
    bool hasVEML7700 = false;
    bool hasHDC1080 = false;

private:
    float readTemperature();
    float readHumidity();
    int readCO2();
    float readLight();
    int readPressure();
    float readBattery();
    SensorReadings readingsStruct;
};

bool getHasHDC1080()
{
    // TODO code to read device and manufacturer ID and return true if a match
    char manufacturerID[5];
    char deviceID[5];
    sprintf(manufacturerID, "%02X", hdc1080.readManufacturerId());
    sprintf(deviceID, "%02X", hdc1080.readDeviceId());
    return (strcmp(manufacturerID, "5449") == 0 && strcmp(deviceID, "1050") == 0);
}

bool getHasSCD30()
{
    return scd30.dataAvailable();
}

bool getHasBMP388()
{
    return !bmp388.begin(); // returns 0 if available
}

bool getHasVEML7700()
{
    return veml7700.begin();
}

void CompositeSensor::begin()
{
    scd30.begin();
    hdc1080.begin(0x40);

    hasSCD30 = getHasSCD30();
    hasBMP388 = getHasBMP388();
    hasVEML7700 = getHasVEML7700();
    hasHDC1080 = getHasHDC1080();

    if (hasSCD30)
    {
        // scd30.begin();
        scd30.setMeasurementInterval(4);
        scd30.setAmbientPressure(1013); // TODO use barometric presssure if available
    }

    if (hasVEML7700)
    {
        veml7700.setGain(VEML7700_GAIN_1);
        veml7700.setIntegrationTime(VEML7700_IT_800MS);
        veml7700.setLowThreshold(10000);
        veml7700.setHighThreshold(20000);
        veml7700.interruptEnable(false);
        delay(100);
    }
}

float CompositeSensor::readTemperature()
{
    if (hasSCD30)
    {
        return Round(scd30.getTemperature(), 1);
    }
    else if (hasHDC1080)
    {
        return Round(hdc1080.readTemperature(), 1);
    }
    else if (hasBMP388)
    {
        return Round(bmp388.readTemperature(), 1);
    }
    else
    {
        return 0.0;
    }
}

float CompositeSensor::readHumidity()
{
    if (hasSCD30)
    {
        return Round(scd30.getHumidity(), 0);
    }
    else if (hasHDC1080)
    {
        return Round(hdc1080.readHumidity(), 0);
    }
    else
    {
        return 0.0;
    }
}

int CompositeSensor::readCO2()
{
    return hasSCD30 ? scd30.getCO2() : 0;
}

float CompositeSensor::readLight()
{
    return hasVEML7700 ? Round(veml7700.readLux(), 0) : 0.0;
}

int CompositeSensor::readPressure()
{
    return hasBMP388 ? bmp388.readPressure() / 100.0 : 0;
}

float CompositeSensor::readBattery()
{
    float measuredvbat = analogRead(VBATPIN);
    measuredvbat *= 2;    // we divided by 2, so multiply back
    measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
    measuredvbat /= 1024; // convert to voltage

    return Round(measuredvbat, 2);
}

CompositeSensor::SensorReadings CompositeSensor::readSensors()
{
    readingsStruct.temp = CompositeSensor::readTemperature();
    readingsStruct.humidity = CompositeSensor::readHumidity();
    readingsStruct.co2 = CompositeSensor::readCO2();
    readingsStruct.light = CompositeSensor::readLight();
    readingsStruct.pressure = CompositeSensor::readPressure();
    readingsStruct.battery = CompositeSensor::readBattery();

    return readingsStruct;
}

double Round(double value, int places)
{
    double factor = pow(10, places);
    return ((int)(value * factor)) / factor;
}