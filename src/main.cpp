#include <Arduino.h>
#include "display.h"
#include "font.h"

Display display;

void setup()
{
    Serial.begin(115200);
    display.Init();
}


unsigned long now = 0;
unsigned long lastUpdate = 0;
unsigned long updateInterval = 100;

int8_t bitPosition = COLS * 8;
uint8_t n = 0;

uint8_t chr[ROWS] = {
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111
};

uint8_t chr2[ROWS] = {
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
};

void loop()
{
    now = millis();
    display.Drive();

    if (now - lastUpdate >= updateInterval)
    {
        display.Clear();
        display.Write(font_6x7[n], bitPosition);
        Serial.println(bitPosition);

        bitPosition--;
        if (bitPosition == -8)
        {
            bitPosition = COLS * 8;

            n++;
            if (n == 10) n = 0;
        }

        lastUpdate = now;
    }
}