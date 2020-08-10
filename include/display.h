
#pragma once

#include <Arduino.h>

#define DATA_SET_DELAY() /*delayMicroseconds(100);*/
#define LATCH_SET_DELAY() /*delayMicroseconds(5);*/
#define LATCH_CLEAR_DELAY() /*delayMicroseconds(5);*/

#define ROWS 8
#define COLS 6


class Display
{
protected:
    static const uint8_t NUM_BUFFERS = 2;
    uint8_t displayingBuffer = 0;
    uint8_t workingBuffer = 1;

    uint8_t buffer[NUM_BUFFERS][ROWS][COLS];

    uint8_t latchMask[2] =
        // PORTB     PORTD
        {0b00111111, 0b10000000}; // row driver 0

    uint8_t colLatchMask[COLS][2] =
    {
        // PORTB     PORTD
        
        
        
        
        
        {0b00010000, 0b00000000}, // col driver 5
        {0b00000000, 0b10000000}, // col driver 4
        {0b00000001, 0b00000000}, // col driver 3
        {0b00000010, 0b00000000}, // col driver 2
        {0b00000100, 0b00000000}, // col driver 1
        {0b00001000, 0b00000000}, // col driver 0
    };

    uint8_t dataMask[2] =
        // PORTC    PORTD
        {0b00111111, 0b01100000};

    uint8_t rowLatchMask[2] =
        // PORTB     PORTD
        {0b00100000, 0b00000000}; // row driver 0

    uint8_t rowMask[ROWS] =
    {
        0b10000000,
        0b01000000,
        0b00100000,
        0b00010000,
        0b00001000,
        0b00000100,
        0b00000010,
        0b00000001,
    };


protected:
    inline void SetLatchCol(uint8_t col)
    {
        PORTB = (PORTB & ~latchMask[0]) | colLatchMask[col][0];
        PORTD = (PORTD & ~latchMask[1]) | colLatchMask[col][1];
    }


    inline void ClearLatchCol(uint8_t col)
    {
        PORTB = (PORTB & ~colLatchMask[col][0]);
        PORTD = (PORTD & ~colLatchMask[col][1]);
    }

    inline void SetLatchRow()
    {
        PORTB = (PORTB & ~latchMask[0]) | rowLatchMask[0];
        PORTD = (PORTD & ~latchMask[1]) | rowLatchMask[1];
    }

    inline void ClearLatchRow()
    {
        PORTB = (PORTB & ~rowLatchMask[0]);
        PORTD = (PORTD & ~rowLatchMask[1]);
    }

    inline void SetData(uint8_t data)
    {
        PORTC = (PORTC & ~dataMask[0]) | (data & dataMask[0]);
        PORTD = (PORTD & ~dataMask[1]) | ((data >> 1) & dataMask[1]);
    }

    inline void SelectRow(uint8_t row)
    {
        SetData(~rowMask[row]);
    }

    inline void DeselectRow()
    {
        SetData(0xFF);
    }


    unsigned long lastRowUpdateMicros = 0ul;
    unsigned long rowUpdateIntervalMicros = ((1000ul * 1000ul) / 60ul /*fps*/) / (ROWS +1);
    uint8_t currentRow = 0;

public:

    void Init()
    {
        // data pins
        DDRC |= dataMask[0];
        DDRD |= dataMask[1];

        // latch pins
        DDRB |= latchMask[0];
        DDRD |= latchMask[1];


        // clear rows
        SetLatchRow();
        LATCH_SET_DELAY();

        SetData(0xFF);
        DATA_SET_DELAY();

        ClearLatchRow();
        LATCH_CLEAR_DELAY();

        // clear columns
        for (uint8_t col = 0; col < COLS; col++)
        {
            SetLatchCol(col);
            LATCH_SET_DELAY();

            SetData(0); 
            DATA_SET_DELAY();

            ClearLatchCol(col);
            LATCH_CLEAR_DELAY();
        }

        memset(buffer, 0, NUM_BUFFERS * ROWS * COLS * sizeof(uint8_t));

        // for (uint8_t r = 0; r < ROWS; r++)
        // {
        //     buffer[currentBuffer][r][0] = (1 << r) | (1 << (ROWS - r - 1));
        // }
    }

    void DisplayRow(uint8_t row)
    {
        // turn of all rows


        DeselectRow();
        DATA_SET_DELAY();

        SetLatchRow();
        LATCH_SET_DELAY();

        ClearLatchRow();
        LATCH_CLEAR_DELAY();

        for (uint8_t col = 0; col < COLS; col++)
        {
            SetData(buffer[displayingBuffer][row][col]); 
            DATA_SET_DELAY();

            SetLatchCol(col);
            LATCH_SET_DELAY();

            ClearLatchCol(col);
            LATCH_CLEAR_DELAY();
        }

        // show current row
        SelectRow(row);
        DATA_SET_DELAY();

        SetLatchRow();
        LATCH_SET_DELAY();

        ClearLatchRow();
        LATCH_CLEAR_DELAY();
    }

    unsigned long nowMicros;
    unsigned long sinceLastRowUpdateMicros;

    bool Drive(unsigned long* _nowMicros)
    {
        nowMicros = *_nowMicros;

        sinceLastRowUpdateMicros = nowMicros - lastRowUpdateMicros;

        if (sinceLastRowUpdateMicros >= rowUpdateIntervalMicros)
        {
            lastRowUpdateMicros = nowMicros;

            if (currentRow < ROWS)
            {
                DisplayRow(currentRow);
            }
            else
            {
                // vsync
            }

            if (currentRow == ROWS)
            {
                currentRow = 0;
            }
            else
            {
                currentRow++;
            }
            return true;
        }
        return false;
    }

    void Clear()
    {
        memset(buffer[workingBuffer], 0, ROWS * COLS * sizeof(uint8_t));
    }

    inline uint8_t* WorkingBuffer()
    {
        return (uint8_t*)(buffer[workingBuffer]);
    }

    inline unsigned long RowIntervalMicros()
    {
        return rowUpdateIntervalMicros;
    }

    void Write(uint8_t data[ROWS], int8_t bitPosition, uint8_t bitWidth = 0)
    {
        int8_t driver = bitPosition / (int8_t)8;
        if (bitPosition < 0) driver--;

        int8_t nextDriver = driver +1;



        int8_t driverPos = bitPosition % 8;
        if (driverPos < 0)
        {
            driverPos = 8 + driverPos;
        }


        // Serial.print("bitpos: ");
        // Serial.print(bitPosition);
        // Serial.print(" driver: ");
        // Serial.print(driver);
        // Serial.print(" driverpos: ");
        // Serial.println(driverPos);

        if (driver >= 0 && driver < COLS)
        {
            for (uint8_t r = 0; r < ROWS; r++)
            {
                uint8_t currDriverData = data[r] << driverPos;
                buffer[workingBuffer][r][driver] |= currDriverData;
            }
        }

        if (nextDriver >= 0 && nextDriver < COLS)
        {
            for (uint8_t r = 0; r < ROWS; r++)
            {
                buffer[workingBuffer][r][nextDriver] |= data[r] >> (8 - driverPos);
            }
        }
    }

    inline bool Vsync()
    {
        return currentRow == ROWS;
    }

    inline void FlipBuffer()
    {
        workingBuffer++;
        if (workingBuffer == NUM_BUFFERS)
        {
            workingBuffer = 0;
        }

        displayingBuffer++;
        if (displayingBuffer == NUM_BUFFERS)
        {
            displayingBuffer = 0;
        }
    }
};