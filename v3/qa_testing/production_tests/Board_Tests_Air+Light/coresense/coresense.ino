#include <Wire.h>
extern TwoWire Wire1;
#include <OneWire.h>
#include "config.cpp"



byte MAC_ID[LENGTH_FORMAT3 + 2] = {ID_MAC, 134,0,0,0,0,0,0}; // MAC address
OneWire ds2401(PIN_DS2401);  //DS2401 PIN
byte Temp_byte[8];
byte pass_flag = 1;
unsigned char Temp_uchar[4] = {'F','A','I','L'};


void initializecoresense(void)
{
    byte i;

    pinMode(PIN_SPV_AMP,INPUT);
    pinMode(PIN_SVP_SPL,INPUT);
    pinMode(PIN_RAW_MIC,INPUT);
    pinMode(PIN_HIH4030,INPUT);
    pinMode(PIN_CHEMSENSE_POW, OUTPUT);
    pinMode(PIN_CHEMSENSE_HBT, INPUT);
    pinMode(PIN_CHEMSENSE_RST, OUTPUT);
    pinMode(PIN_HBT, OUTPUT);


    if (ds2401.reset() == TRUE)
    {
        ds2401.write(0x33);
        for (i = 0; i < 8; i++)
        {
            Temp_byte[i] = ds2401.read();
        }

        if (OneWire::crc8(Temp_byte, 8) == 0)
        {
            for (i=1; i<7; i++)
            {
                MAC_ID[i + 1] = Temp_byte[i];
            }

        }

        else
        {

            MAC_ID[3] = 0xff;

        }
    }
    else //Nothing is connected in the bus
    {
        MAC_ID[3] = 0xaa;
    }
}

void writeEEPROM (unsigned int memory_address, byte data_byte )
{
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((int)(memory_address >> 8));   // MSB
    Wire.write((int)(memory_address & 0xFF)); // LSB
    Wire.write(data_byte);
    Wire.endTransmission();
    delay(5);
}

byte readEEPROM (unsigned int memory_address )
{
    byte recv_data = 0xff;

    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((int)(memory_address >> 8));   // MSB
    Wire.write((int)(memory_address & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(EEPROM_ADDRESS,1);

    if (Wire.available())
    {
        recv_data = Wire.read();
    }
    return recv_data;
}


void setup()
{
    delay(2000);
    Wire.begin();
    delay(2000);
    SerialUSB.begin(115200);
    Serial3.begin(115200);
    initializecoresense();
}


void loop()
{
    SerialUSB.println(">>>>>> Core Sense Testing - Test 1 <<<<<<");
    SerialUSB.println(" ");
    SerialUSB.println("1. Power-ON : PASS");
    SerialUSB.println(" ");
    SerialUSB.println("2. Atmel Power & Communication: PASS ");
    SerialUSB.println(" ");
    SerialUSB.print("3. I2C Bus Test : ");

    for (byte i=0; i<LENGTH_FORMAT3; i++)
    {
        writeEEPROM (i,MAC_ID[i+2]);
    }

    delay(100);

    for (byte i=0; i<LENGTH_FORMAT3; i++)
    {
        if (readEEPROM(i) != MAC_ID[i+2])
        {
            pass_flag = 0;
            break;
        }
    }

    if (pass_flag == 1)
    {
        SerialUSB.println("PASS");
    }
    else
    {
        SerialUSB.println("FAIL");
    }
    SerialUSB.println(" ");
    SerialUSB.print("4. Unique Board ID - ");

    for (byte i=2; i<8; i++)
    {
        SerialUSB.print(MAC_ID[i],HEX);

        if (i < 7)
        {
            SerialUSB.print(":");
        }
        else
        {
            SerialUSB.print("\n");
        }
    }
    SerialUSB.println(" ");
    SerialUSB.print("5. Airsense-Lightsense Connectivity Test : ");
    if (Wire.requestFrom(0x27,1) == 0x01) // we are checking if we can read the HIH6130 chip present on the Lightsense board from the Airsense.
    {
        SerialUSB.println("PASS.");
    }

    else
    {
        SerialUSB.println("FAIL");
    }
    SerialUSB.println(" ");
    SerialUSB.print("6. Airsense-Chemsense Serial Connectivity Test : ");
    Serial3.write("P");
    delay(100);
    Temp_uchar[0] = Serial3.read();
    Serial3.write("A");
    delay(100);
    Temp_uchar[1] = Serial3.read();
    Serial3.write("S");
    delay(100);
    Temp_uchar[2] = Serial3.read();
    Serial3.write("S");
    delay(100);
    Temp_uchar[3] = Serial3.read();
    for (byte i=0; i<4; i++)
    {
        SerialUSB.write(Temp_uchar[i]);
    }
    SerialUSB.println(" ");
    SerialUSB.println(" ");
    SerialUSB.print("7. Airsense-Chemsense Heartbeat and Reset Test : ");

    digitalWrite(PIN_CHEMSENSE_RST, HIGH);
    delay(100);
    if (digitalRead(PIN_CHEMSENSE_HBT) == 1)
    {
        delay(100);
        digitalWrite(PIN_CHEMSENSE_RST, LOW);
        delay(100);
        if (digitalRead(PIN_CHEMSENSE_HBT) == 0)
        {
            SerialUSB.println("PASS");
            SerialUSB.println(" ");
            SerialUSB.println("8. Powercontrol Test : (The LED on the ACHEM breakout board should keep blinking) ");
            SerialUSB.println(" ");
            SerialUSB.println("9. Heartbeat Test : The test has PASSED if the LED blinks when attached to PIN 3 and PIN 4 of the header in LightSense, else it has FAILED. ");
            SerialUSB.println(" ");
            SerialUSB.println(">>>>>> Test Finished <<<<<<");

            while (1)
            {
                digitalWrite(PIN_CHEMSENSE_POW, LOW);
                digitalWrite(PIN_HBT, LOW);
                delay(1000);
                digitalWrite(PIN_HBT, HIGH);
                digitalWrite(PIN_CHEMSENSE_POW, HIGH);
                delay(1000);
            }
        }
    }

    SerialUSB.println("FAIL");
    SerialUSB.println(" ");
    SerialUSB.println("8. Powercontrol Test : The test has PASSED if the LED on the ACHEM board is blinking, else it has FAILED. ");
    SerialUSB.println(" ");
    SerialUSB.println("9. Heartbeat Test : The test has PASSED if the LED blinks when attached to PIN 3 and PIN 4 of the header in LightSense, else it has FAILED. ");
    SerialUSB.println(" ");
    SerialUSB.println(" ");
    SerialUSB.println(">>>>>> Test Finished <<<<<<");

    while (1)
    {
      digitalWrite(PIN_CHEMSENSE_POW, LOW);
      digitalWrite(PIN_HBT, LOW);
      delay(1000);
      digitalWrite(PIN_HBT, HIGH);
      digitalWrite(PIN_CHEMSENSE_POW, HIGH);
      delay(1000);
    }

}





