#include <Arduino.h>
#include "display.h"
#include "timedisplay.h"
#include "font.h"

Display display;
TimeDisplay timeDisplay(&display);



unsigned long nowMicros = 0;
unsigned long prev = 0;
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

void setup()
{
    Serial.begin(115200);
    display.Init();
    nowMicros = micros();
}

void loop()
{
    nowMicros = micros();


    bool didwork = display.Drive(&nowMicros);
    if (didwork)
    {
        uint8_t t = timeDisplay.TaskPump(&nowMicros);

        unsigned long duration = micros() - nowMicros;

        // Serial.print("Task ");
        // Serial.print(t);
        // Serial.print(" took ");
        // Serial.print(duration);
        // Serial.print("/");
        // Serial.print(display.RowIntervalMicros());
        // Serial.print(": ");
        // if (duration >= display.RowIntervalMicros())
        // {
        //     Serial.print("TOO LONG");
        // }
        // else
        // {
        //     Serial.print("OK");
        // }

        // Serial.println();


    }
}