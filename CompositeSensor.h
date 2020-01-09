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
#include "SparkFun_SCD30_Arduino_Library.h"
#include "Adafruit_VEML7700.h"
#include "DFRobot_BMP388_I2C.h"
#include "ClosedCube_HDC1080.h"

#define VBATPIN A9 // for measuring battery voltage

SCD30 scd30;
Adafruit_VEML7700 veml7700 = Adafruit_VEML7700();
DFRobot_BMP388_I2C bmp388;
ClosedCube_HDC1080 hdc1080;

class CompositeSensor
{
public:
    void begin();
    float readTemperature();
    float readHumidity();
    int readCO2();
    float readLight();
    int readPressure();
    float readBattery();

    bool hasSCD30 = false;
    bool hasBMP388 = false;
    bool hasVEML7700 = false;
    bool hasHDC1080 = false;
};

bool getHasHDC1080()
{
    // TODO code to read device and manufacturer ID and return true if a match
    return false;
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
    hasSCD30 = getHasSCD30();
    hasBMP388 = getHasBMP388();
    hasVEML7700 = getHasVEML7700();
    hasHDC1080 = getHasHDC1080();

    if (hasSCD30)
    {
        scd30.setMeasurementInterval(4);
        scd30.setAmbientPressure(1013); // TODO use barometric presssure if available
    }

    if (hasBMP388) // returns 0 if initialised
    {
        // inititialisation goes here
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
        return scd30.getTemperature();
    }
    else if (hasHDC1080)
    {
        return hdc1080.readTemperature();
    }
    else if (hasBMP388)
    {
        return bmp388.readTemperature();
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
        return scd30.getHumidity();
    }
    else if (hasHDC1080)
    {
        return hdc1080.readHumidity();
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
    return hasVEML7700 ? veml7700.readLux() : 0.0;
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

    return measuredvbat;
}