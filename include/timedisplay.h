#pragma once
#include <Arduino.h>
#include "display.h"
#include "font.h"

class TimeDisplay
{
protected:
    Display* display;

    enum Task : uint8_t
    {
        SPLIT_MINUTES = 0,
        SPLIT_SECONDS,
        SPLIT_HUNDREDS,

        SPLIT_NUMBERS,
        
        DRAW_MINUTES,
        DRAW_SECONDS,
        DRAW_MILLIS,

        WAIT_VSYNC,

        NUM_TASKS
    };

    Task currentTask = Task::SPLIT_MINUTES;
    inline void NextTask()
    {
        currentTask = (Task)(currentTask + 1);
        if (currentTask == Task::NUM_TASKS)
        {
            currentTask = Task::SPLIT_MINUTES;
        }
    }



    unsigned long timeMillis = 0;

    static const unsigned long MILLIS_IN_MINUTE = 60ul * 1000ul;
    static const unsigned long MILLIS_IN_SECOND = 1000ul;
    //static const unsigned long MILLIS_IN_HUNDREDS = 10ul;


    unsigned long splitTime = 0;
    unsigned long splitMinutes = 0;
    unsigned long splitSeconds = 0;
    unsigned long splitHundreds = 0;

    uint8_t minutes = 0;
    uint8_t seconds = 0;
    uint16_t milliseconds = 0;

    uint8_t minutes_1 = 0;
    uint8_t minutes_2 = 0;

    uint8_t seconds_1 = 0;
    uint8_t seconds_2 = 0;

    uint8_t milliseconds_1 = 0;
    uint8_t milliseconds_2 = 0;
    uint8_t milliseconds_3 = 0;

    


    inline void TwoDigits(uint8_t n, uint8_t* digit1, uint8_t* digit2)
    {
        *digit1 = (n / 10);

        n -= (*digit1 * 10);
        *digit2 = n;
    }

    inline void ThreeDigits(uint16_t n, uint8_t* digit1, uint8_t* digit2, uint8_t *digit3)
    {
        uint16_t d1 = n / 100;
        n -= d1 * 100;

        uint16_t d2 = n / 10;
        n -= d2 * 10;

        *digit1 = d1;
        *digit2 = d2;
        *digit3 = n;
    }

    static const int8_t TOTAL_WIDTH = (COLS -1) * 8;
    int8_t pos = 0;

public:
    TimeDisplay(Display* display)
    {
        this->display = display;
    }

    uint8_t TaskPump(unsigned long* nowMicros)
    {
        uint8_t t = (uint8_t)currentTask;

        switch (currentTask)
        {
            case Task::SPLIT_MINUTES:
                
                timeMillis = *nowMicros / 1000;

                splitTime = timeMillis;

                splitMinutes = splitTime / MILLIS_IN_MINUTE;
                splitTime -= splitMinutes * MILLIS_IN_MINUTE;

                minutes = splitMinutes;

                NextTask();
            break;

            case Task::SPLIT_SECONDS:
                splitSeconds = splitTime / MILLIS_IN_SECOND;
                splitTime -= splitSeconds * MILLIS_IN_SECOND;

                seconds = splitSeconds;

                NextTask();
            break;

            case Task::SPLIT_HUNDREDS:
                milliseconds = splitTime;

                NextTask();
            break;

            case Task::SPLIT_NUMBERS:
                TwoDigits(minutes, &minutes_1, &minutes_2);
                TwoDigits(seconds, &seconds_1, &seconds_2);
                ThreeDigits(milliseconds, &milliseconds_1, &milliseconds_2, &milliseconds_3);
                NextTask();
            break;

            case Task::DRAW_MINUTES:
                // reset buffer
                display->Clear();
                pos = -1;


                display->Write(font_6x7[minutes_1], TOTAL_WIDTH - pos);
                pos += 6;

                display->Write(font_6x7[minutes_2], TOTAL_WIDTH - pos);
                pos += 6;

                NextTask();
            break;

            case Task::DRAW_SECONDS:
                display->Write(collon, TOTAL_WIDTH - pos);
                pos += 3;

                display->Write(font_6x7[seconds_1], TOTAL_WIDTH - pos);
                pos += 6;

                display->Write(font_6x7[seconds_2], TOTAL_WIDTH - pos);
                pos += 6;

                NextTask();
            break;

            case Task::DRAW_MILLIS:
                display->Write(dot, TOTAL_WIDTH - pos);
                pos += 3;

                display->Write(font_6x7[milliseconds_1], TOTAL_WIDTH - pos);
                pos += 6;

                // display->Write(font_6x7[milliseconds_2], TOTAL_WIDTH - pos);
                // pos += 6;

                // display->Write(font_6x7[milliseconds_3], TOTAL_WIDTH - pos);
                // pos += 6;

                display->Write(dash, TOTAL_WIDTH - pos);
                pos += 6;

                display->Write(dash, TOTAL_WIDTH - pos);
                pos += 6;

                NextTask();
            break;


            case Task::WAIT_VSYNC:
                if (display->Vsync())
                {
                    display->FlipBuffer();
                    NextTask();
                }
            break;
        }

        return t;
    }
};