#include <Arduino.h>
#include "font.h"


#define ROWS 8
#define COLS 6

uint8_t buff[ROWS][COLS] = {0};

uint8_t latchMask[2] =
    // PORTB     PORTD
    {0b00111111, 0b10000000}; // row driver 0

uint8_t colLatchMask[COLS][2] =
{
    // PORTB     PORTD
    {0b00001000, 0b00000000}, // col driver 0
    {0b00000100, 0b00000000}, // col driver 1
    {0b00000010, 0b00000000}, // col driver 2
    {0b00000001, 0b00000000}, // col driver 3
    {0b00000000, 0b10000000}, // col driver 4
    {0b00010000, 0b00000000}, // col driver 5
};

// uint8_t dataMask[8][2] =
// {
//     // PORTC     PORTD
//     {0b00000001, 0b00000000}, // data bit 0
//     {0b00000010, 0b00000000}, // data bit 1
//     {0b00000100, 0b00000000}, // data bit 2
//     {0b00001000, 0b00000000}, // data bit 3
//     {0b00010000, 0b00000000}, // data bit 4
//     {0b00000000, 0b00000000}, // data bit 4
//     {0b00000000, 0b00110000} // data bit 4
// };

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

uint8_t row = 0;

#define DELAY_WRITE() delay(100);
#define DELAY_LATCH() delay(10);

void printBinary(byte inByte)
{
    for (int b = 7; b >= 0; b--)
    {
        Serial.print(bitRead(inByte, b));
    }
}

void printLatch(const char *msg)
{
    Serial.println(msg);
    printBinary(PORTB);
    Serial.print(' ');
    printBinary(PORTD);
    Serial.println();
}

inline void SetLatchCol(uint8_t col)
{
    PORTB = (PORTB & latchMask[0]) | colLatchMask[col][0];
    PORTD = (PORTD & latchMask[1]) | colLatchMask[col][1];
}

inline void ClearLatchCol(uint8_t col)
{
    PORTB = (PORTB & latchMask[0]) & ~colLatchMask[col][0];
    PORTD = (PORTD & latchMask[1]) & ~colLatchMask[col][1];
}

inline void SetLatchRow()
{
    PORTB = (PORTB & latchMask[0]) | rowLatchMask[0];
    PORTD = (PORTD & latchMask[1]) | rowLatchMask[1];
}

inline void ClearLatchRow()
{
    PORTB = (PORTB & latchMask[0]) & ~rowLatchMask[0];
    PORTD = (PORTD & latchMask[1]) & ~rowLatchMask[1];
}

inline void SetData(uint8_t data)
{
    PORTC = (PORTD & ~dataMask[0]) | (data & dataMask[0]);
    PORTD = (PORTD & ~dataMask[1]) | ((data >> 1) & dataMask[1]);
}

void DisplayRow(uint8_t row)
{
    // turn of all rows
    SetLatchRow();
    SetData(0);
    delayMicroseconds(100);
    ClearLatchRow();


    for (uint8_t col = 0; col < COLS; col++)
    {
        SetLatchCol(col);
        SetData(buff[row][col]); 
        delayMicroseconds(100);
        ClearLatchCol(col);
    }

    // show current row
    SetLatchRow();
    SetData(rowMask[row]);
    delayMicroseconds(100);
    ClearLatchRow();
}

void DisplayScreen()
{
    for (uint8_t r = 0; r < ROWS; r++)
    {
        DisplayRow(r);
    }
}

#define BIT_MASK(__TYPE__, __ONE_COUNT__) \
    ((__TYPE__) (-((__ONE_COUNT__) != 0))) \
    & (((__TYPE__) -1) >> ((sizeof(__TYPE__) * CHAR_BIT) - (__ONE_COUNT__)))

void Write(int x, uint8_t bitmap[])
{
    uint8_t col = x / 8;
    for (uint8_t row = 0; row < ROWS; row++)
    {
        buff[row][col] = bitmap[row];
        printBinary(bitmap[row]);
        Serial.println();
    }

    Serial.println();
}


void ClearBuff()
{
    memset(buff, 0, ROWS * COLS * sizeof(uint8_t));
}

void SetBuff()
{
    memset(buff, 0xFF, ROWS * COLS * sizeof(uint8_t));
}


void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);

    // data pins
    DDRC |= dataMask[0];
    DDRD |= dataMask[1];

    // latch pins
    DDRB |= latchMask[0];
    DDRD |= latchMask[1];



    SetData(0xFF);

    uint8_t data = 0xff;
    uint8_t c = data & dataMask[0];
    uint8_t d = (data >> 2) & dataMask[1];

    Serial.print("C: ");
    printBinary(c);
    Serial.print(" D: ");
    printBinary(d);
    Serial.println();
    
    ClearBuff();
    DisplayScreen();

    Serial.println("Arduino ready");



    // fill some pattern
    for (uint8_t r = 0; r < ROWS; r++)
    {
        buff[r][0] = 2;

        // for (uint8_t c = 0; c < COLS; c++)
        // {
        //     buff[r][c] = 1 << r;

        // }
    }
    
}

uint8_t col = 0;
uint8_t bit = 0;

unsigned long lastUpdate = 0;
bool full = false;
uint8_t n = 0;

unsigned long FPS = 1ul;
unsigned long lastRowUpdate = 0ul;
unsigned long rowUpdateMicros = 1000ul * 1000ul / FPS / ROWS; 

unsigned long frameCount = 0;
unsigned long measuredFps = 0;

unsigned long lastFpsMeasurement = 0;
unsigned long measuePeriodMicros = 5000ul * 1000ul;
unsigned long fpsTimeFactor = measuePeriodMicros / 1000ul / 1000ul;




void loop()
{
    //return;
    unsigned long now = micros();

    if (now - lastRowUpdate >= rowUpdateMicros)
    {
        DisplayRow(row);
        row++;
        if (row == ROWS) 
        {
            row = 0;
            frameCount++;
        }

        lastRowUpdate = now;
    }

    if (now - lastFpsMeasurement >= measuePeriodMicros)
    {
        measuredFps = frameCount / fpsTimeFactor;
        Serial.print("FPS: ");
        Serial.println(measuredFps);

        lastFpsMeasurement = now;
        frameCount = 0;
    }
    
    if (now - lastUpdate >= 500ul * 1000ul)
    {

        //bit++;
        // if (bit == 8)
        // {
        //     bit = 0;
        //     col++;
        //     if (col == COLS)
        //     {
        //         col = 0;
        //     }
        // }

        // ClearBuff();

        // uint8_t mask = 1 << (8 - bit);
        // for (uint8_t r = 0; r < ROWS; r++)
        // {
        //     buff[r][col] = mask;
        // }




        // full = !full;

        // if (full)
        // {
        //     SetBuff();
        // }
        // else
        // {
        //     ClearBuff();
        // }






        lastUpdate = now;
    }
}
