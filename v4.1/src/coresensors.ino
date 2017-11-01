
// Metsense
void ReadMacAdd(byte *sensorReading, int readinglength)
{
	SerialUSB.print("printing mac address");;
	byte id[8];

	if (!ds2401.reset()) 
	{
		SerialUSB.print("device is not ready");
		// return any sign that this device is not ready!!!
		return;
	}

	ds2401.write(0x33);

	for (int i = 0; i < 8; i++)
		id[i] = ds2401.read();

	if (OneWire::crc8(id, 8) != 0)
	{
		SerialUSB.print("failed to pass crc check");
		// return any sign that it is failed to checi crc
		return;
	}

	for (int i = 0; i < 8; i++) 
	{
		byte h = (id[i] >> 4) & 0x0f;
		byte l = (id[i] >> 0) & 0x0f;
		SerialUSB.println(h, HEX);
		SerialUSB.println(l, HEX);

		sensorReading[i] = (id[i] >> 4) & 0x0f;
		sensorReading[i + 1] = (id[i] >> 0) & 0x0f;

		readinglength += 2;
	}
}

void ReadTMP112(byte *sensorReading, int readinglength)
{
	SerialUSB.print("TMP112");

	const byte TMP112_TEMP_REG = 0x00;

	byte writebyte[1] = {TMP112_TEMP_REG};
	byte readarray[2];
	WriteI2C(TMP112_ADDRESS, 1, writebyte);
	delay(100);
	ReadI2C(TMP112_ADDRESS, 2, sensorReading);

	sensorReading[0] = readarray[0];
	sensorReading[1] = readarray[1];
	readinglength = 2;
}

void ReadHTU21D(byte *sensorReading, int readinglength)
{
	SerialUSB.print("HTU21D");

	const byte TRIGGER_TEMP_MEASURE_NOHOLD = 0xF3;
	const byte TRIGGER_HUMD_MEASURE_NOHOLD = 0xF5;

	// Read Temprature, Hang out while measurement is taken 50mS max
	byte readarray[3];
	byte writebyte[1] = {TRIGGER_TEMP_MEASURE_NOHOLD};
	WriteReadI2C(HTDU21D_ADDRESS, 1, writebyte, 3, readarray, 55);

	for (int i = 0; i < 3; i++)
		sensorReading[i] = readarray[i];
	
	//Request a humidity reading
	writebyte[0] = TRIGGER_HUMD_MEASURE_NOHOLD;
	WriteReadI2C(HTDU21D_ADDRESS, 1, writebyte, 3, readarray, 55);

	for (int i = 0; i < 3; i++)
		sensorReading[i + 3] = readarray[i];

	readinglength = 6;
}

void ReadHIH4030(byte *sensorReading, int readinglength)
{
	int value = analogRead(HIH4030_PIN);

	sensorReading[0] = (value >> 8) & 0xFF;
	sensorReading[1] = value & 0xFF;

	readinglength = 2;
}

void ReadBMP180(byte *sensorReading, int readinglength)
{
	const byte BMP180_REGISTER_CONTROL = 0xF4;
	const byte BMP180_REGISTER_TEMPDATA = 0xF6;
	const byte BMP180_REGISTER_PRESSUREDATA = 0xF6;
	const byte BMP180_REGISTER_READTEMPCMD = 0x2E;
	const byte BMP180_REGISTER_READPRESSURECMD = 0x34;

	// Read temprature
	byte writearray[2] = {BMP180_REGISTER_CONTROL, BMP180_REGISTER_READTEMPCMD};
	WriteI2C(BMP180_ADDRESS, 2, writearray);
	delay(5);
	byte writebyte[1] = {BMP180_REGISTER_TEMPDATA};
	byte readarray[2];
	WriteReadI2C(BMP180_ADDRESS, 1, writebyte, 2, readarray);

	for (int i = 0; i < 2; i++)
		sensorReading[i] = readarray[i];

	// Read pressure
	byte _bmp180Mode = 3;
	writebyte[0] = BMP180_REGISTER_CONTROL;
	writebyte[1] = BMP180_REGISTER_READPRESSURECMD + (_bmp180Mode << 6);
	WriteI2C(BMP180_ADDRESS, 2, writebyte);
	delay(26);
	writebyte[0] = BMP180_REGISTER_PRESSUREDATA;
	WriteReadI2C(BMP180_ADDRESS, 1, writebyte, 2, readarray);
	writebyte[0] = BMP180_REGISTER_PRESSUREDATA + 2;
	byte readbyte[1];
	WriteReadI2C(BMP180_ADDRESS, 1, writebyte, 1, readbyte);

	for (int i = 0; i < 2; i++)
		sensorReading[i + 2] = readarray[i];
	sensorReading[4] = readbyte[0];

	readinglength = 5;
}

void ReadPR103J2(byte *sensorReading, int readinglength)
{
	int PR = analogRead(PRJ103J2_PIN);

	sensorReading[0] = (PR >> 8) & 0xFF;
	sensorReading[1] = PR & 0xFF;

	readinglength = 2;
}

void ReadTSL250ms(byte *sensorReading, int readinglength)
{
	int TSL250_1 = analogRead(TSL250RDms_PIN);

	sensorReading[0] = (TSL250_1 >> 8) & 0xFF;
	sensorReading[1] = TSL250_1 & 0xFF;

	readinglength = 2;
}

void ReadMMA(byte *sensorReading, int readinglength)
{
	const byte OUT_X_MSB = 0x01;
	const byte XYZ_DATA_CFG = 0x0E;
	const byte CTRL_REG1 = 0x2A;

	byte readarray[6];  // x/y/z accel register data stored here
	byte writebyte[1] = {OUT_X_MSB};
	WriteReadI2C(MMA8452_ADDRESS, 1, writebyte, 6, readarray, false);

	for (int i = 0; i < 6; i++)
		sensorReading[i] = readarray[i];

	readinglength = 6;
}

void ReadSPV(byte *sensorReading, int readinglength)
{
	// long buff[128];
	int buff = 0;
	for (int i = 0; i < 128; i++)
	{
		buff = analogRead(SPV_RAW_MIC_PIN);
		sensorReading[i] = (buff >> 8) & 0xFF;
		sensorReading[i + 1] = buff & 0xFF;
		readinglength += 2;
	}
}

void ReadTSYS01(byte *sensorReading, int readinglength)
{
	const byte TSYS01StartReg = 0x48; //commands sensor to begin measurement / output calculation
	const byte TSYS01TempReg = 0x00; //requests most recent output from measurement

	byte writebyte[1] = {TSYS01StartReg};
	WriteI2C(TSYS01_ADDRESS, 1, writebyte);
	delay(10);
	writebyte[0] = {TSYS01TempReg};
	byte readarray[3];
	WriteReadI2C(TSYS01_ADDRESS, 1, writebyte, 3, readarray);
	// the last read is not used

	for (int i = 0; i < 3; i++)
		sensorReading[i] = readarray[i];
	readinglength = 3;
}

// Lightsense
void ReadHMC(byte *sensorReading, int readinglength)
{
	const byte HMC5883_REGISTER_MAG_OUT_X_H_M = 0x03;

	byte writebyte[1] = {HMC5883_REGISTER_MAG_OUT_X_H_M};
	byte readarray[6];
	WriteReadI2C(HMC5883_ADDESS, 1, writebyte, 6, readarray);

	for (int i = 0; i < 6; i++)
		sensorReading[i] = readarray[i];
	readinglength = 6;
}

void ReadHIH6130(byte *sensorReading, int readinglength)
{
	byte readarray[4];
	WriteReadI2C(HIH6130_ADDRESS, 4, readarray, 100);

	for (int i = 0; i < 4; i++)
		sensorReading[i] = readarray[i];
	readinglength = 4;
	
	// Temp_byte[0] = (Temp_byte[1] >> 6) & 0x03;

	// Temp_byte[1] = Temp_byte[1] & 0x3f;
	// Temp_int[0] = (((unsigned int)Temp_byte[1]) << 8) | Temp_byte[2];

	// Temp_int[1] = (((unsigned int)Temp_byte[3]) << 8) | Temp_byte[4];
	// Temp_int[1] = Temp_int[1] / 4;

	// Temp_float[1] = (float) Temp_int[0] * 6.10e-3;
	// Temp_float[0] = (float) Temp_int[1] * 1.007e-2 - 40.0;
}

void ReadAPDS(byte *sensorReading, int readinglength)
{
	// byte address = mcp3428_2.returnAddress();
	byte writebyte[1] = {mcp3428_2.returnRegister(MCP342X::CHANNEL_0)};
	byte readarray[3];
	WriteReadI2C(MCP3428_2_ADDRESS, 1, writebyte, 3, readarray);

	for (int i = 0; i < 3; i++)
		sensorReading[i] = readarray[i];
	readinglength = 3;

	// the last read is not used

	// selectChannel(MCP342X::CHANNEL_0, MCP342X::GAIN_1);
	// int APDS = mcp3428_2.readADC();
}

void ReadTSL260RD(byte *sensorReading, int readinglength)
{
	// byte address = mcp3428_1.returnAddress();
	byte writebyte[1] = {mcp3428_1.returnRegister(MCP342X::CHANNEL_1)};
	byte readarray[3];
	WriteReadI2C(MCP3428_1_ADDRESS, 1, writebyte, 3, readarray);

	for (int i = 0; i < 3; i++)
		sensorReading[i] = readarray[i];
	readinglength = 3;

	// the last read is not used

	// mcp3428_1.selectChannel(MCP342X::CHANNEL_1, MCP342X::GAIN_1);
	// int TSL260 = mcp3428_1.readADC();
}

void ReadTSL250RDls(byte *sensorReading, int readinglength)
{
	// byte address = mcp3428_1.returnAddress();
	byte writebyte[1] = {mcp3428_1.returnRegister(MCP342X::CHANNEL_3)};
	byte readarray[3];
	WriteReadI2C(MCP3428_1_ADDRESS, 1, writebyte, 3, readarray);

	for (int i = 0; i < 3; i++)
		sensorReading[i] = readarray[i];
	readinglength = 3;

	// the last read is not used

	// mcp3428_1.selectChannel(MCP342X::CHANNEL_3, MCP342X::GAIN_1);
	// int TSL250_2 = mcp3428_1.readADC();
}

void ReadMLX(byte *sensorReading, int readinglength)
{
	// byte address = mcp3428_1.returnAddress();
	byte writebyte[1] = {mcp3428_1.returnRegister(MCP342X::CHANNEL_0)};
	byte readarray[3];
	WriteReadI2C(MCP3428_1_ADDRESS, 1, writebyte, 3, readarray);

	for (int i = 0; i < 3; i++)
		sensorReading[i] = readarray[i];
	readinglength = 3;

	// the last read is not used

	// mcp3428_1.selectChannel(MCP342X::CHANNEL_0, MCP342X::GAIN_1);
	// int MLX = mcp3428_1.readADC();
}

void ReadML8511(byte *sensorReading, int readinglength)
{
	// byte address = mcp3428_1.returnAddress();
	byte writebyte[1] = {mcp3428_1.returnRegister(MCP342X::CHANNEL_2)};
	byte readarray[3];
	WriteReadI2C(MCP3428_1_ADDRESS, 1, writebyte, 3, readarray);

	for (int i = 0; i < 3; i++)
		sensorReading[i] = readarray[i];
	readinglength = 3;

	// the last read is not used

	// mcp3428_1.selectChannel(MCP342X::CHANNEL_2, MCP342X::GAIN_1);
	// int ML8511 = mcp3428_1.readADC();
}

void ReadTMP421(byte *sensorReading, int readinglength)
{
	byte readbyte[1];
	
	// high-byte
	byte writebyte[1] = {0x00};
	WriteReadI2C(TMP421_ADDRESS, 1, writebyte, 1, readbyte);

	sensorReading[0] = readbyte[0];
	readinglength += 1;

	// low-byte
	writebyte[0] = 0x10;
	WriteReadI2C(TMP421_ADDRESS, 1, writebyte, 1, readbyte);

	sensorReading[1] = readbyte[0];
	readinglength += 1;

	// byte testdata[2];
	// WriteI2C(TMP421_ADDRESS, char(0x00));
	// ReadI2C(TMP421_ADDRESS, testdata[0]); // high-byte

	// WriteI2C(TMP421_ADDRESS, char(0x10));
	// ReadI2C(TMP421_ADDRESS, testdata[1]); // low-byte
}


// Chemsense
void ReadChem(byte *sensorReading, int readinglength)
{
	ReadRS232(sensorReading, readinglength);
}


// // Alphasense
// byte alphaReading[256];
// void ReadAlphaHisto()
// {
// 	SPI.beginTransaction(set1);
// 	digitalWrite(33, LOW);
// 	delay(100);

// 	SPI.transfer(0x30);   // 0xF3
// 	delay(100);

// 	for (i = 0; i < 62; i++)
// 	{
// 		alphaReading[i] = SPI.transfer(0x30);
// 		delay(100);
// 	}

// 	digitalWrite(33, HIGH);
// 	SPI.endTransaction();	
// }

// void ReadAlphaSerial()
// {
// 	SPI.beginTransaction(set1);
// 	digitalWrite(33, LOW);
// 	delay(100);

// 	SPI.transfer(0x10);    // 0xF3
// 	delay(100);

// 	for (i = 0; i < 20; i++)
// 	{
// 	    alphaReading[i] = SPI.transfer(0x10);
// 	    delay(100);
// 	}

// 	digitalWrite(33, HIGH);
// 	SPI.endTransaction();
// }

// void ReadAlphaFWver()
// {
// 	SPI.beginTransaction(set1);
// 	digitalWrite(33, LOW);
// 	delay(100);

// 	SPI.transfer(0x12);    // 0xF3
// 	delay(100);

// 	alphaReading[0] = SPI.transfer(0x12);
// 	delay(100);
// 	alphaReading[1] = SPI.transfer(0x12);
// 	delay(100);

// 	digitalWrite(33, HIGH);
// 	SPI.endTransaction();
// }

// void ReadAlphaConfig()
// {
// 	SPI.beginTransaction(set1);
// 	digitalWrite(33, LOW);
// 	delay(100);

// 	SPI.transfer(0x3C);   // 0xF3
// 	delay(100);

// 	for (i = 0; i < 256; i++)
// 	{
// 		alphaReading[i] = SPI.transfer(0x3C);
// 		delay(100);
// 	}

// 	digitalWrite(33, HIGH);
// 	SPI.endTransaction();

// }

// void ReadOnSet()
// {

// }

// void ReadDecagon()
// {

// }