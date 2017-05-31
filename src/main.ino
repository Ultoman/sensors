#include <Arduino.h>
#include <Wire.h>
#include <OneWire.h>
#include "scanner.h"

// NOTE May want to model system as simple state machine to recall
// what we're supposed to be doing at any given time.

const char *version = "coresense 4.0.0";

char inputbuf[256];
int inputlen;

bool startswith(const char *s, const char *p) {
    while (*p != '\0') {
        if (*s != *p) {
            return false;
        }

        s++;
        p++;
    }

    return true;
}

const byte BBEGIN = 0xFC;
const byte BEND = 0xFE;
const byte BESCAPE = 0xCC;

bool ControlByte(byte b) {
    return b == BBEGIN || b == BEND || b == BESCAPE;
}

void SendFrame(byte *b, int n) {
    byte crc = 0;

    SerialUSB.write(BBEGIN);

    for (int i = 0; i < n; i++) {
        if (ControlByte(b[i])) {
            SerialUSB.write(BESCAPE);
            SerialUSB.write(b[i] ^ 0x20);
        } else {
            SerialUSB.write(b[i]);
        }
    }

    SerialUSB.write(crc);
    SerialUSB.write(BEND);
}

bool matches(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

void setup() {
    SerialUSB.begin(9600);

    while (!SerialUSB) {
    }

    Wire.begin();
}

OneWire ds(48);


const char *commandVersion(int argc, const char **argv) {
    SerialUSB.println(version);
    return NULL;
}

const char *commandID(int argc, const char **argv) {
    byte mac[8];

    ds.reset();
    ds.write(0x33);

    for (int i = 0; i < 8; i++) {
        mac[i] = ds.read();
    }

    if (OneWire::crc8(mac, 8) != 0) {
        return "failed crc";
    }

    for (int i = 0; i < 8; i++) {
        SerialUSB.print((mac[i] >> 4) & 0x0F, HEX);
        SerialUSB.print((mac[i] >> 0) & 0x0F, HEX);
    }

    SerialUSB.println();
    return NULL;
}

const char *commandPinMode(int argc, const char **argv) {
    if (argc != 3) {
        return "invalid args";
    }

    int pin = atoi(argv[1]);
    int mode;

    if (matches(argv[2], "input")) {
        mode = INPUT;
    } else if (matches(argv[2], "output")) {
        mode = OUTPUT;
    } else {
        return "invalid pin mode";
    }

    pinMode(pin, mode);
    return NULL;
}

const char *commandPinWrite(int argc, const char **argv) {
    if (argc != 3) {
        return "invalid args";
    }

    int pin = atoi(argv[1]);
    int value = atoi(argv[2]);
    digitalWrite(pin, value);
    return NULL;
}

const char *commandI2CBegin(int argc, const char **argv) {
    if (argc != 2) {
        return "invalid args";
    }

    int addr = atoi(argv[1]);
    Wire.beginTransmission(addr);
    return NULL;
}

const char *commandI2CEnd(int argc, const char **argv) {
    Wire.endTransmission();
    return NULL;
}

const char *commandI2CReq(int argc, const char **argv) {
    return "not implemented";
}

const char *commandI2CRead(int argc, const char **argv) {
    return "not implemented";
}

struct {
    const char *name;
    const char *(*func)(int argc, const char **argv);
} commands[] = {
    {"ver", commandVersion},
    {"id", commandID},
    {"pin-mode", commandPinMode},
    {"pin-write", commandPinWrite},
    {"i2c-begin", commandI2CBegin},
    {"i2c-end", commandI2CEnd},
    {"i2c-req", commandI2CReq},
    {"i2c-read", commandI2CRead},
};

const int numcommands = sizeof(commands) / sizeof(commands[0]);

const char *dispatchcommand(Scanner &scanner) {
    if (scanner.argc == 0) {
        return NULL;
    }

    for (int i = 0; i < numcommands; i++) {
        if (matches(scanner.argv[0], commands[i].name)) {
            return commands[i].func(scanner.argc, (const char **)scanner.argv);
        }
    }

    return "invalid command";
}

const char *processinput() {
    const char *err;

    Scanner scanner(inputbuf, 256);

    scanner.Split();

    if (scanner.Err() != NULL) {
        return scanner.Err();
    }

    err = dispatchcommand(scanner);

    if (err != NULL) {
        return err;
    }

    // ack message
    SerialUSB.print("ok:");

    for (int i = 0; i < scanner.argc; i++) {
        SerialUSB.print(" ");
        SerialUSB.print(scanner.argv[i]);
    }

    SerialUSB.println();

    return NULL;
}

// in general, we should be able to sleep / power off unless we need to do
// any processing.
//
// one of the items that can be added is using a timer interrupt with a few
// second delay to read data out. this allows us to have a built in timeout
// system.
void loop() {
    // byte frame[] = {1, 2, 3, 4, 5};
    // SendFrame(frame, 5);
    // delay(2500);

    SerialUSB.print("> ");

    inputbuf[0] = '\0';
    inputlen = 0;

    while (1) {
        if (inputlen >= 256) {
            SerialUSB.println("error: buffer overflow");
            break;
        }

        while (SerialUSB.available() == 0) {
        }

        char b = SerialUSB.read();

        if (isprint(b)) {
            inputbuf[inputlen++] = b;
            inputbuf[inputlen] = '\0';
        }

        if (b == '\r' || b == '\n') {
            SerialUSB.println();
            processinput();
            break;
        } else {
            SerialUSB.write(b);
        }
    }
}
