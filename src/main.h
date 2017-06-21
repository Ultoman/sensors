#include <Arduino.h>
#include <Wire.h>
// #include <OneWire.h>
#include "scanner.h"
#include "stringutils.h"
#include "fmt.h"

#include "./util/findSensor.h"
#include "./met/met.h"
#include "./light/light.h"
#include "./chem/chem.h"
// OneWire ds(48);

#define PRINTF_BUF 256

#define PIN_CHEMSENSE_POW 47
#define CHEMSENSE_DATARATE 115200

#define MetLightMAC 3087011
#define MetSenNum 0x09
#define LightSenNum 0x08

int NumVal = 0;
char dataReading[PRINTF_BUF];

Scanner scanner;
CSensor sensor;
CMet cmet;
CLight clight;
Cchem chem;