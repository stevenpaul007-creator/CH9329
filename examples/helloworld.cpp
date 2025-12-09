#include "Arduino.h"
#include "CH9329.h"

CH9329 *CH9329Client;

struct CH9329CFG ch9329cfgs[CH9329COUNT] = {
{
    .led_pin = 2,
    .rx_pin = 8,
    .tx_pin = 9,
    .cfg1_pin = 10,
    .mode1_pin = 11,
    .addr = 0x30,
    .baud = 115200,
    .CFG1 = HIGH,
    .MODE1 = HIGH // LOW: 注：Linux/Android/macOS等操作系统下，建议使用该模式。
},
{
    .led_pin = 28,
    .rx_pin = 8,
    .tx_pin = 9,
    .cfg1_pin = 27,
    .mode1_pin = 26,
    .addr = 0x01,
    .baud = 115200,
    .CFG1 = HIGH,
    .MODE1 = HIGH // LOW: 注：Linux/Android/macOS等操作系统下，建议使用该模式。
}};

void setup()
{

    delay(1000);

    int index = 0;

    CH9329Client = new CH9329(&Serial2, ch9329cfgs);
    CH9329Client.pressASCII(index, 'H');
    CH9329Client.releaseAll(index);

    char *str = "Hello World";
    CH9329Client.sendString(index, str, 11);

    CH9329Client->mouseMove(index, 2, 2, 0, 0);

    CH9329Client->mouseMoveAbs(index, 2, 2, 0, 0);
    
    CH9329Client->releaseAll(index);
    CH9329Client->mouseRelease(index);
}

void loop()
{
  CH9329Client->readUart();
}