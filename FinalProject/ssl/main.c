//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - Game Bundle
// Application Overview - This contains a maze and hangman game bundle
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup i2c_demo
//! @{
//
//*****************************************************************************
// Standard includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

// Simplelink includes
#include "simplelink.h"

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_nvic.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "utils.h"
#include "systick.h"
#include "rom_map.h"
#include "spi.h"
#include "uart.h"

// Common interface includes
#include "uart_if.h"
#include "gpio.h"
#include "oled_test.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"
#include "i2c_if.h"
#include "utils/network_utils.h"
#include "common.h"

// Pin configurations
#include "pin_mux_config.h"

//*****************************************************************************
//                      MACRO DEFINITIONS - START
//*****************************************************************************
//*****************************************************************************
//                 SYSTICK MACROS -- Start
//*****************************************************************************
// some helpful macros for systick:

// the cc3200's fixed clock frequency of 80 MHz
// note the use of ULL to indicate an unsigned long long constant
#define SYSCLKFREQ 80000000ULL

// macro to convert ticks to microseconds
#define TICKS_TO_US(ticks) \
    ((((ticks) / SYSCLKFREQ) * 1000000ULL) + \
    ((((ticks) % SYSCLKFREQ) * 1000000ULL) / SYSCLKFREQ))\

// macro to convert microseconds to ticks
#define US_TO_TICKS(us) ((SYSCLKFREQ / 1000000ULL) * (us))

// systick reload value set to 30ms period
// (PERIOD_SEC) * (SYSCLKFREQ) = PERIOD_TICKS
#define SYSTICK_RELOAD_VAL 3200000UL

// track systick counter periods elapsed
// if it is not 0, we know the transmission ended
volatile int systick_cnt = 0;
volatile int systick_cnt2 = 0;
//*****************************************************************************
//                 SYSTICK MACROS -- End
//*****************************************************************************
//*****************************************************************************
//                      AWS MACROS
//*****************************************************************************
//NEED TO UPDATE THIS FOR IT TO WORK!
#define DATE                4    /* Current Date */
#define MONTH               6     /* Month 1-12 */
#define YEAR                2024  /* Current year */
#define HOUR                8    /* Time - hours */
#define MINUTE              0    /* Time - minutes */
#define SECOND              0     /* Time - seconds */

#define APPLICATION_NAME      "SSL"
#define APPLICATION_VERSION   "SQ24"
#define SERVER_NAME           "a1a04qgvrgklcm-ats.iot.us-west-1.amazonaws.com" // CHANGE ME (CHANGED)
#define GOOGLE_DST_PORT       8443

#define POSTHEADER "POST /things/Daniel_CC3200_Board/shadow HTTP/1.1\r\n"             // CHANGE ME (CHANGED)
#define HOSTHEADER "Host: a1a04qgvrgklcm-ats.iot.us-west-1.amazonaws.com\r\n"  // CHANGE ME (CHANGED)
#define CHEADER "Connection: Keep-Alive\r\n"
#define CTHEADER "Content-Type: application/json; charset=utf-8\r\n"
#define CLHEADER1 "Content-Length: "
#define CLHEADER2 "\r\n\r\n"

#define GETHEADER "GET /things/Daniel_CC3200_Board/shadow HTTP/1.1\r\n"     // CHANGE ME (CHANGED)

//*****************************************************************************
//                      I2C MACROS
//*****************************************************************************

#define UART_PRINT              Report
#define FOREVER                 1
#define CONSOLE                 UARTA0_BASE
#define FAILURE                 -1
#define SUCCESS                 0
#define RETERR_IF_TRUE(condition) {if(condition) return FAILURE;}
#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
                                   if (SUCCESS != iRetVal) \
                                     return  iRetVal;}

//*****************************************************************************
//                      I2C MACROS - END
//*****************************************************************************

//*****************************************************************************
//                      SPI MACROS
//*****************************************************************************

#define SPI_IF_BIT_RATE  100000

//*****************************************************************************
//                      SPI MACROS - END
//*****************************************************************************

//*****************************************************************************
//                      ACCELEROMETER MACROS
//*****************************************************************************

#define ACCELEROMETER_ADDRESS  0x18UL          // Register offset for the accelerometer
#define XAXISREGISTEROFFSET   0x03UL          // Register offset for the X axis on the accelerometer
#define YAXISREGISTEROFFSET   0x05UL          // Register offset for the Y axis on the accelerometer

//*****************************************************************************
//                      ACCELEROMETER MACROS - END
//*****************************************************************************

//*****************************************************************************
//                      CURSOR MACROS
//*****************************************************************************

#define MAZECURSORX  91
#define MAZECURSORY  23

#define HANGCURSORX  80
#define HANGCURSORY  43

#define HIGHCURSORX  97
#define HIGHCURSORY  63

#define EASYCURSORX  49
#define EASYCURSORY  38

#define MEDCURSORX  73
#define MEDCURSORY  69

#define HARDCURSORX  49
#define HARDCURSORY  98

//*****************************************************************************
//                      CURSOR MACROS - END
//*****************************************************************************

//*****************************************************************************
//                      IR REMOTE MACROS
//*****************************************************************************

#define DIGITZERO   551487735UL      // 32 bit decimal representation of a 0 being transmitted
#define DIGITONE    551520375UL      // 32 bit decimal representation of a 1 being transmitted
#define DIGITTWO    551504055UL      // 32 bit decimal representation of a 2 being transmitted
#define DIGITTHREE  551536695UL      // 32 bit decimal representation of a 3 being transmitted
#define DIGITFOUR   551495895UL      // 32 bit decimal representation of a 4 being transmitted
#define DIGITFIVE   551528535UL      // 32 bit decimal representation of a 5 being transmitted
#define DIGITSIX    551512215UL      // 32 bit decimal representation of a 6 being transmitted
#define DIGITSEVEN  551544855UL      // 32 bit decimal representation of a 7 being transmitted
#define DIGITEIGHT  551491815UL      // 32 bit decimal representation of a 8 being transmitted
#define DIGITNINE   551524455UL      // 32 bit decimal representation of a 9 being transmitted
#define DIGITLAST   551508135UL      // 32 bit decimal representation of LAST being transmitted
#define DIGITMUTE   551522415UL      // 32 bit decimal representation of MUTE being transmitted

//*****************************************************************************
//                      IR REMOTE MACROS - END
//*****************************************************************************

//*****************************************************************************
//                      MAZE MACROS
//*****************************************************************************

#define ACCEL_AVERAGE_WINDOW 5

//*****************************************************************************
//                      MAZE MACROS - END
//*****************************************************************************

//*****************************************************************************
//                      GAME MACROS
//*****************************************************************************
#define MAXSCORE 800

#define MAXHIGHSCORE 0

//*****************************************************************************
//                      GAME MACROS - END
//*****************************************************************************

//*****************************************************************************
//                      MACRO DEFINITIONS - END
//*****************************************************************************

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************

//*****************************************************************************
//                      BOARD VARIABLES
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

//*****************************************************************************
//                      BOARD VARIABLES - END
//*****************************************************************************

//*****************************************************************************
//                      GAME VARIABLES
//*****************************************************************************
static int score = MAXSCORE;
static int currentHighScore = 0;

typedef enum {
    MAIN_MENU_STATE,
    CURSOR_STATE,
    MAZE_DIFFICULTY_SCREEN,         // Define state constants
    MAZE_STATE,
    HIGHSCORE_STATE,
    HANGMAN_SCREEN,
    HANGMAN_STATE,
    WIN_LOSE_SCREEN
} GameState;

static GameState currentState;
static GameState previousState;

static char genericMaze[SSD1351HEIGHT][SSD1351WIDTH] = {0};

static char hangman[SSD1351HEIGHT][SSD1351WIDTH] = {0};

static int gameResult = 0;
//*****************************************************************************
//                      GAME VARIABLES - END
//*****************************************************************************

//*****************************************************************************
//                      INTERRUPT VARIABLES
//*****************************************************************************

volatile unsigned char IR_intflag;          // Lock for the interrupts
volatile unsigned long IR_intcount;         // Keep track of interrupts

volatile unsigned long delta_us;
volatile unsigned long dataStream;          // 32 bit data transmission with 16 address bits and 16 data bits

volatile unsigned long bitCounter;          // global variable to track received bits

volatile int checkFlag = 0;                 //
//*****************************************************************************
//                      INTERRUPT VARIABLES - END
//*****************************************************************************

//*****************************************************************************
//                      CURSOR VARIABLES
//*****************************************************************************

typedef struct cursorCoor {    // struct to move the cursor
    int xCoor;
    int yCoor;
} cursorCoor;

static cursorCoor currentCursor = {.xCoor= MAZECURSORX, .yCoor = MAZECURSORY }; // cursor Initialization

static int currentCursorLocation = 0; // Location of cursor

static int mainCursorArray[2][3] = {
  { MAZECURSORX, HANGCURSORX, HIGHCURSORX },
  { MAZECURSORY, HANGCURSORY, HIGHCURSORY }
};

static int mazeDifCursorArray[2][3] = {
  { EASYCURSORX, MEDCURSORX, HARDCURSORX },
  { EASYCURSORY, MEDCURSORY, HARDCURSORY }
};

// Array of cursors location for letters in hangman
int cursorsArray[5][2] = {{50, 90}, {60, 90}, {70, 90}, {80, 90}, {90, 90}};
//*****************************************************************************
//                      CURSOR VARIABLES - END
//*****************************************************************************

//*****************************************************************************
//                      PIN VARIABLES
//*****************************************************************************

typedef struct PinSetting {         // struct for pins given to us in the TA example
    unsigned long port;
    unsigned int pin;
} PinSetting;

static PinSetting IRpin = { .port = GPIOA0_BASE, .pin = 0x1};   // pin 50 is the IR output pin

//*****************************************************************************
//                      PIN VARIABLES - END
//*****************************************************************************

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************

//****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS
//****************************************************************************

//*****************************************************************************
//
//  AWS Functions
//
//*****************************************************************************
//*****************************************************************************
//
//! Parse the JSON string for the high score
//!
//! \param  jsonString is the JSON string to be parsed
//!
//! \return value is the value extracted from the JSON string
//!
//*****************************************************************************
char *getScore(const char *jsonString) {
    const char *key = "\"var\":\"";
    char *start = strstr(jsonString, key);

    if (start == NULL) {
        // Key not found
        return NULL;
    }

    // Move the pointer to the start of the value
    start += strlen(key);

    // Find the end of the value (next occurrence of the quote)
    char *end = strchr(start, '\"');
    if (end == NULL) {
        // Malformed JSON string
        return NULL;
    }

    // Calculate the length of the value
    int length = end - start;

    // Allocate memory for the value string (+1 for null terminator)
    char *value = (char *)malloc((length + 1) * sizeof(char));
    if (value == NULL) {
        // Memory allocation failed
        return NULL;
    }

    // Copy the value into the new string
    strncpy(value, start, length);
    // Null-terminate the string
    value[length] = '\0';

    return value;
}

//*****************************************************************************
//
//! Generate the JSON string to be sent using Http_post()
//!
//! \param  string is the string to be added to the JSON string
//!
//! \return jsonString is the JSON string created
//!
//*****************************************************************************
char *generateJSON(const char *string) {
    // Calculate the length of the JSON string
    int length = strlen(string) + strlen("{\"state\": {\r\n\"desired\" : {\r\n\"var\" :\"%s\"\r\n}}}\r\n\r\n") + 1;

    // Allocate memory for the JSON string
    char *jsonString = (char *)malloc(length * sizeof(char));
    if (jsonString == NULL) {
        // Memory allocation failed
        return NULL;
    }

    // Construct the JSON string
    sprintf(jsonString, "{\"state\": {\r\n\"desired\" : {\r\n\"var\" :\"%s\"\r\n}}}\r\n\r\n", string);

    return jsonString;
}

//*****************************************************************************
//
//! Generate the proper AWS format to communicate with the shadow
//!
//! \param  iTLSSockID is return value from the TLS connection
//! \param  data is the message to be sent across to AWS
//!
//! \return 0 for success, -1 for failure
//!
//*****************************************************************************
static int http_post(int iTLSSockID, const char* data) {
    char acSendBuff[512];
    char acRecvbuff[1460];
    char cCLLength[200];
    char* pcBufHeaders;
    int lRetVal = 0;

    pcBufHeaders = acSendBuff;
    strcpy(pcBufHeaders, POSTHEADER);
    pcBufHeaders += strlen(POSTHEADER);
    strcpy(pcBufHeaders, HOSTHEADER);
    pcBufHeaders += strlen(HOSTHEADER);
    strcpy(pcBufHeaders, CHEADER);
    pcBufHeaders += strlen(CHEADER);
    strcpy(pcBufHeaders, "\r\n\r\n");

    int dataLength = strlen(data);

    strcpy(pcBufHeaders, CTHEADER);
    pcBufHeaders += strlen(CTHEADER);
    strcpy(pcBufHeaders, CLHEADER1);

    pcBufHeaders += strlen(CLHEADER1);
    sprintf(cCLLength, "%d", dataLength);

    strcpy(pcBufHeaders, cCLLength);
    pcBufHeaders += strlen(cCLLength);
    strcpy(pcBufHeaders, CLHEADER2);
    pcBufHeaders += strlen(CLHEADER2);

    strcpy(pcBufHeaders, data);
    pcBufHeaders += strlen(data);

    int testDataLength = strlen(acSendBuff);

    UART_PRINT(acSendBuff);

    // Send the packet to the server
    lRetVal = sl_Send(iTLSSockID, acSendBuff, strlen(acSendBuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("POST failed. Error Number: %i\n\r",lRetVal);
        sl_Close(iTLSSockID);

        return lRetVal;
    }
    lRetVal = sl_Recv(iTLSSockID, &acRecvbuff[0], sizeof(acRecvbuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("Received failed. Error Number: %i\n\r",lRetVal);

        return lRetVal;
    }
    else {
        acRecvbuff[lRetVal+1] = '\0';
        UART_PRINT(acRecvbuff);
        UART_PRINT("\n\r\n\r");
    }

    return 0;
}

//*****************************************************************************
//
//! Communicates with AWS and returns the high score
//!
//! \param  iTLSSockID is return value from the TLS connection
//!
//! \return resultScore is the score from the parsed JSON
//!
//*****************************************************************************
static int http_get(int iTLSSockID){
    char acSendBuff[512];
    char acRecvbuff[1460];
    char* pcBufHeaders;
    int lRetVal = 0;

    pcBufHeaders = acSendBuff;
    strcpy(pcBufHeaders, GETHEADER);
    pcBufHeaders += strlen(GETHEADER);
    strcpy(pcBufHeaders, HOSTHEADER);
    pcBufHeaders += strlen(HOSTHEADER);
    strcpy(pcBufHeaders, CHEADER);
    pcBufHeaders += strlen(CHEADER);
    strcpy(pcBufHeaders, "\r\n\r\n");


    UART_PRINT(acSendBuff);

    //
    // Send the packet to the server */
    //
    lRetVal = sl_Send(iTLSSockID, acSendBuff, strlen(acSendBuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("GET failed. Error Number: %i\n\r",lRetVal);
        sl_Close(iTLSSockID);

        return lRetVal;
    }
    lRetVal = sl_Recv(iTLSSockID, &acRecvbuff[0], sizeof(acRecvbuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("Received failed. Error Number: %i\n\r",lRetVal);
        //sl_Close(iSSLSockID);

        return lRetVal;
    }
    else {
        acRecvbuff[lRetVal+1] = '\0';
        UART_PRINT(acRecvbuff);
        UART_PRINT("\n\r\n\r");
    }
    int resultScore;
    char *scoreStr = getScore(acRecvbuff);
        if (scoreStr != NULL) {
            // Convert the extracted string to an integer
            resultScore = atoi(scoreStr);
            // Free the allocated memory
            free(scoreStr);
        } else {
            printf("Score not found or error in parsing\n");
        }

    return resultScore;
}

//*****************************************************************************
//
//! This function updates the date and time of CC3200.
//!
//! \param None
//!
//! \return
//!     0 for success, negative otherwise
//!
//*****************************************************************************
static int set_time() {
    long retVal;

    g_time.tm_day = DATE;
    g_time.tm_mon = MONTH;
    g_time.tm_year = YEAR;
    g_time.tm_sec = HOUR;
    g_time.tm_hour = MINUTE;
    g_time.tm_min = SECOND;

    retVal = sl_DevSet(SL_DEVICE_GENERAL_CONFIGURATION,
                          SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
                          sizeof(SlDateTime),(unsigned char *)(&g_time));

    ASSERT_ON_ERROR(retVal);
    return SUCCESS;
}

//*****************************************************************************
//
//  AWS Functions - END
//
//*****************************************************************************

//*****************************************************************************
//
//  SYSTICK Functions
//
//*****************************************************************************

/*
 * Reset SysTick Counter
 */
static inline void SysTickReset(void) {
    // any write to the ST_CURRENT register clears it
    // after clearing it automatically gets reset without
    // triggering exception logic
    // see reference manual section 3.2.1
    HWREG(NVIC_ST_CURRENT) = 1;

    // clear the global count variable
    systick_cnt = 0;
}

/**
 * SysTick Interrupt Handler
 *
 * Keep track of whether the systick counter wrapped
 */
static void SysTickHandler(void) {
    // increment every time the systick handler fires
    systick_cnt++;
    //printf("Systick_cnt2: %lu\n", systick_cnt2);
}

//*****************************************************************************
//
//  SYSTICK Functions - END
//
//*****************************************************************************

//*****************************************************************************
//
//  Interrupt Functions
//
//*****************************************************************************

//*****************************************************************************
//
//! Handles interrupts on falling edges and assembles dataStream based on time between interrupts
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************

static void IRIntHandler(void) { // IR Handler

    unsigned long ulStatus,delta;

    IR_intflag=1;
    ulStatus = MAP_GPIOIntStatus(IRpin.port, true);
    MAP_GPIOIntClear(IRpin.port, ulStatus);        // clear interrupts on the IR pin

    delta = SYSTICK_RELOAD_VAL - SysTickValueGet();
    // convert elapsed cycles to microseconds
    delta_us = TICKS_TO_US(delta);

    if ((delta_us > 13300)&&(delta_us <13600)){
        dataStream = 0;
        bitCounter = 0;
        IR_intcount++;
    }
    else if(bitCounter > 31){
        SysTickReset();
        IR_intcount++;
        checkFlag = 1;
        return;
    }
    else if(delta_us <= 1200){
        dataStream = dataStream << 1;
        bitCounter++;
        IR_intcount++;
    }
    else if((delta_us > 1200)&&(delta_us <2700)){
        dataStream = dataStream << 1;
        dataStream = dataStream + 1;
        bitCounter++;
        IR_intcount++;
    }

    SysTickReset();

}

//*****************************************************************************
//
//  Interrupt Functions - END
//
//*****************************************************************************
//*****************************************************************************
//
//  Cursor Functions
//
//*****************************************************************************
//*****************************************************************************
//
//! Draws a Cursor
//!
//! \param  x is the x-coordinate of the beginning of the point of the cursor
//! \param  y is the y-coordinate of the beginning of the point of the cursor
//! \param  color is the desired color of the cursor
//!
//! \return None
//!
//*****************************************************************************
void drawCursor(int x, int y, unsigned int color){
    drawPixel(x,y,color);
    drawPixel((x+1),(y-1),color);
    drawPixel((x+1),(y+1),color);          // Draws the head
    drawPixel((x+2),(y-2),color);
    drawPixel((x+2),(y+2),color);

    drawPixel((x+2),y,color);
    drawPixel((x+3),y,color);           // Draws the tail
    drawPixel((x+4),y,color);
    drawPixel((x+5),y,color);
}
//*****************************************************************************
//
//! Draws the inverse of the cursor
//!
//! \param  x is the x-coordinate of the beginning of the point of the cursor
//! \param  y is the y-coordinate of the beginning of the point of the cursor
//!
//! \return None
//!
//*****************************************************************************
void blink(int x, int y){
    drawCursor(x,y,BLACK);
    fillRect(x,(y-2),6,5, WHITE);
}
//*****************************************************************************
//
//! Draws the cursor and pauses then draws a white rectangle to replicate a blink
//!
//! \param  x is the x-coordinate of the beginning of the point of the cursor
//! \param  y is the y-coordinate of the beginning of the point of the cursor
//! \param  color is the desired color of the cursor
//!
//! \return None
//!
//*****************************************************************************
void blinkCursor(int x, int y, unsigned int color){
    fillRect(x,(y-2),6,5, BLACK);
    drawCursor(x,y,color);
    UtilsDelay(10000000);
    blink(x,y);
    fillRect(x,(y-2),6,5, BLACK);
}
//*****************************************************************************
//
//! Uses the array to update our cursor global variables
//!
//! \param  array is the cursor array of the current screen
//! \param  bounds is the number of options on the current screen
//!
//! \return None
//!
//*****************************************************************************

void changeCursorLocation(int array[2][3],int bounds){
    if(dataStream == DIGITTWO){
        if(currentCursorLocation == 0){
            currentCursorLocation = bounds;
        }
        else{
            currentCursorLocation--;
        }
    }
    else{
        if(currentCursorLocation == bounds){
            currentCursorLocation = 0;
        }
        else{
            currentCursorLocation++;
        }
    }
    currentCursor.xCoor = array[0][currentCursorLocation];
    currentCursor.yCoor = array[1][currentCursorLocation];
}

//*****************************************************************************
//
//  Cursor Functions - END
//
//*****************************************************************************

//*****************************************************************************
//
//  Maze Construction Functions
//
//*****************************************************************************
//*****************************************************************************
//
//! Draw a straight line using Bresenham's algorithm from wikipedia
//! https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#:~:text=Bresenham's%20line%20algorithm%20is%20a,straight%20line%20between%20two%20points.
//!
//! \param  maze is the selected maze
//! \param  startRow is the start row for the line
//! \param  startCol is the start column for the line
//! \param  endRow is the end row for the line
//! \param  endCol is the end column for the line
//! \param  value is character to be placed in a straight line
//!
//! \return None
//!
//*****************************************************************************

void mazeLine(char maze[SSD1351HEIGHT][SSD1351WIDTH], int startRow, int startCol, int endRow, int endCol, char value) {
    int deltaX = abs(endCol - startCol);
    int deltaY = abs(endRow - startRow);
    int stepX = (startCol < endCol) ? 1 : -1;
    int stepY = (startRow < endRow) ? 1 : -1;
    int error = deltaX - deltaY;

    while (true) {
        maze[startRow][startCol] = value;
        if (startRow == endRow && startCol == endCol) break;
        int error2 = error * 2;
        if (error2 > -deltaY) {
            error -= deltaY;
            startCol += stepX;
        }
        if (error2 < deltaX) {
            error += deltaX;
            startRow += stepY;
        }
    }
}

//*****************************************************************************
//
//! Build the Easy Maze
//!
//! \param  maze is the selected maze
//! \param  value is character to be placed in a straight line
//!
//! \return None
//!
//*****************************************************************************

void buildEasyMaze(char maze[SSD1351HEIGHT][SSD1351WIDTH],char value) {
    mazeLine(maze, 113, 109, 113, 128,value);
    mazeLine(maze, 112, 114, 14, 114,value);
    mazeLine(maze, 14, 114, 14, 20,value);
    mazeLine(maze, 14, 20, 27, 20,value);
    mazeLine(maze, 27, 20, 27, 89,value);
    mazeLine(maze, 27, 89, 41, 89,value);
    mazeLine(maze, 41, 89, 41, 20,value);
    mazeLine(maze, 41, 20, 65, 20,value);
    mazeLine(maze, 65, 20, 65, 44,value);
    mazeLine(maze, 65, 44, 56, 44,value);
    mazeLine(maze, 55, 33, 55, 89,value);
    mazeLine(maze, 113, 1, 113, 87,value);
    mazeLine(maze, 112, 20, 77, 20,value);
    mazeLine(maze, 77, 20, 77, 87,value);
    mazeLine(maze, 94, 20, 94, 87,value);
    mazeLine(maze, 64, 73, 64, 103,value);
    mazeLine(maze, 56, 73, 64, 73,value);
}

//*****************************************************************************
//
//! Build the Medium Maze
//!
//! \param  maze is the selected maze
//! \param  value is character to be placed in a straight line
//!
//! \return None
//!
//*****************************************************************************

void buildMediumMaze(char maze[SSD1351HEIGHT][SSD1351WIDTH],char value) {
    mazeLine(maze, 17, 15, 17, 77,value);
    mazeLine(maze, 17, 77, 37, 77,value);
    mazeLine(maze, 37, 77, 37, 15,value);
    mazeLine(maze, 37, 15, 108, 15,value);
    mazeLine(maze, 108, 15, 108, 36,value);
    mazeLine(maze, 108, 36, 118, 36,value);
    mazeLine(maze, 91, 51, 77, 51,value);
    mazeLine(maze, 63, 51, 53, 51,value);
    mazeLine(maze, 53, 51, 53, 93,value);
    mazeLine(maze, 127, 115, 66, 115,value);
    mazeLine(maze, 66, 115, 66, 64,value);
    mazeLine(maze, 53, 93, 30, 93,value);
    mazeLine(maze, 30, 93, 30, 115,value);
    mazeLine(maze, 30, 115, 45, 115,value);
    mazeLine(maze, 45, 115, 45, 127,value);
    mazeLine(maze, 17, 92, 17, 127,value);
    mazeLine(maze, 38, 34, 92, 34,value);
    mazeLine(maze, 92, 34, 92, 100,value);
    mazeLine(maze, 92, 100, 127, 100,value);
    mazeLine(maze, 118, 52, 108, 52,value);
    mazeLine(maze, 108, 52, 108, 100,value);
    mazeLine(maze, 78, 52, 78, 103,value);
}

//*****************************************************************************
//
//! Build the Hard Maze
//!
//! \param  maze is the selected maze
//! \param  value is character to be placed in a straight line
//!
//! \return None
//!
//*****************************************************************************

void buildHardMaze(char maze[SSD1351HEIGHT][SSD1351WIDTH], char value) {
    mazeLine(maze, 12, 29, 12, 22,value);
    mazeLine(maze, 6, 22, 12, 22,value);
    mazeLine(maze, 6, 22, 6, 8,value);
    mazeLine(maze, 6, 8, 25, 8,value);
    mazeLine(maze, 25, 8, 25, 29,value);
    mazeLine(maze, 25, 29, 32, 29,value);
    mazeLine(maze, 32, 29, 32, 90,value);
    mazeLine(maze, 32, 90, 7, 90,value);
    mazeLine(maze, 7, 90, 7, 112,value);
    mazeLine(maze, 0, 80, 14, 80,value);
    mazeLine(maze, 13, 68, 31, 68,value);
    mazeLine(maze, 31, 52, 9, 52,value);
    mazeLine(maze, 9, 52, 9, 43,value);
    mazeLine(maze, 9, 43, 0, 43,value);
    mazeLine(maze, 33, 41, 49, 41,value);
    mazeLine(maze, 33, 65, 50, 65,value);
    mazeLine(maze, 25, 127, 25, 107,value);
    mazeLine(maze, 35, 0, 35, 14,value);
    mazeLine(maze, 35, 14, 93, 14,value);
    mazeLine(maze, 93, 14, 93, 48,value);
    mazeLine(maze, 43, 26, 76, 26,value);
    mazeLine(maze, 64, 26, 64, 88,value);
    mazeLine(maze, 64, 88, 49, 88,value);
    mazeLine(maze, 49, 88, 49, 127,value);
    mazeLine(maze, 48, 102, 37, 102,value);
    mazeLine(maze, 37, 102, 37, 109,value);
    mazeLine(maze, 47, 54, 63, 54,value);
    mazeLine(maze, 49, 78, 84, 78,value);
    mazeLine(maze, 81, 39, 81, 61,value);
    mazeLine(maze, 106, 61, 75, 61,value);
    mazeLine(maze, 75, 61, 75, 69,value);
    mazeLine(maze, 75, 69, 92, 69,value);
    mazeLine(maze, 92, 69, 92, 86,value);
    mazeLine(maze, 92, 86, 78, 86,value);
    mazeLine(maze, 78, 86, 78, 95,value);
    mazeLine(maze, 63, 119, 110, 119,value);
    mazeLine(maze, 110, 119, 110, 127,value);
    mazeLine(maze, 64, 108, 86, 108,value);
    mazeLine(maze, 86, 108, 86, 100,value);
    mazeLine(maze, 86, 100, 106, 100,value);
    mazeLine(maze, 106, 100, 106, 52,value);
    mazeLine(maze, 106, 80, 119, 80,value);
    mazeLine(maze, 119, 52, 119, 108,value);
    mazeLine(maze, 98, 108, 127, 108,value);
    mazeLine(maze, 94, 39, 106, 39,value);
    mazeLine(maze, 106, 39, 106, 14,value);
    mazeLine(maze, 116, 18, 127, 18,value);
    mazeLine(maze, 107, 35, 118, 35,value);
}

//*****************************************************************************
//
//! Iterate through the given array and draw a pixel whenever an X is encountered in the maze
//!
//! \param  maze is the selected maze to be drawn
//!
//! \return None
//!
//*****************************************************************************
void drawMaze(char array[SSD1351HEIGHT][SSD1351WIDTH]){
    int y,x;
    fillScreen(BLACK);
    for (y = 0; y < SSD1351HEIGHT; y++) {
        for (x = 0; x < SSD1351WIDTH; x++) {
            if (array[y][x] == 'x') {
                drawPixel(x, y,WHITE); // Call drawPixel with the current coordinates
            }
        }
    }
}

//*****************************************************************************
//
//! Iterate through the given array and place a 0 at every location in the array
//!
//! \param  maze is the selected maze to be drawn
//!
//! \return None
//!
//*****************************************************************************

void clearMaze(char maze[SSD1351HEIGHT][SSD1351WIDTH]) {
    int i;
    for (i = 0; i < SSD1351HEIGHT; i++) {
        int j;
        for (j = 0; j < SSD1351WIDTH; j++) {
            maze[i][j] = 0;
        }
    }
}

//*****************************************************************************
//
//  Maze Construction Functions - END
//
//*****************************************************************************

//*****************************************************************************
//
//  Menu Selection Functions
//
//*****************************************************************************

//*****************************************************************************
//
//! Draws the main menu
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************
void mainMenu(){
    fillScreen(BLACK);
    setCursor(36,2);
    setTextSize(1);
    setTextColor(WHITE,BLACK);
    Outstr("Main Menu");
    drawLine(35,13,89,13,WHITE);
    drawLine(0,93,127,93,WHITE);
    setCursor(0,95);
    setTextColor(YELLOW,BLACK);
    Outstr("2 to move up");
    setCursor(0,105);
    Outstr("5 to move down");
    setCursor(0,115);
    Outstr("MUTE for Enter/Proceed");

    setCursor(36,20);
    setTextColor(RED,BLACK);
    Outstr("MAZE GAME");
    setCursor(36,40);
    setTextColor(CYAN,BLACK);
    Outstr("HANGMAN");
    setCursor(36,60);
    setTextColor(MAGENTA,BLACK);
    Outstr("HIGHSCORES");

}

//*****************************************************************************
//
//! Draws the maze difficulty select screen
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************

void mazeDifficultySelect(){
    fillScreen(BLACK);
    setTextSize(1);
    setCursor(15,2);
    setTextColor(WHITE,BLACK);
    Outstr("Choose Difficulty");
    setTextSize(2);
    setTextColor(GREEN,BLACK);
    setCursor(0,30);
    Outstr("Easy");
    setCursor(0,60);
    setTextColor(YELLOW,BLACK);
    Outstr("Medium");
    setCursor(0,90);
    setTextColor(RED,BLACK);
    Outstr("Hard");
}

//*****************************************************************************
//
//! draws the high score screen
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************

void highScoresScreen(){
    fillScreen(BLACK);
    setTextSize(2);
    setCursor(5,0);
    setTextColor(YELLOW,BLACK);
    drawLine(0,16,127,16,WHITE);
    Outstr("HighScores");
    setTextSize(1);
    setCursor(15,20);
    setTextColor(RED,BLACK);
    Outstr("Mazes");
    setTextSize(1);
    setCursor(80,20);
    setTextColor(CYAN,BLACK);
    Outstr("HangMan");
}

//*****************************************************************************
//
//! Builds the hangman Screen
//!
//! \param  maze is the array of the maze
//!
//! \return None
//*****************************************************************************

void buildHangman(char value){
    mazeLine(hangman, 123, 6, 123, 49, value);
    mazeLine(hangman, 6, 27, 122, 27, value);
    mazeLine(hangman, 6, 89, 21, 89, value);
    mazeLine(hangman, 6, 27, 6, 89, value);
    mazeLine(hangman, 86, 33, 86, 47, value);
    mazeLine(hangman, 86, 51, 86, 65, value);
    mazeLine(hangman, 86, 69, 86, 83, value);
    mazeLine(hangman, 86, 87, 86, 101, value);
    mazeLine(hangman, 86, 105, 86, 119, value);
    mazeLine(hangman, 94, 53, 127, 53, value);
    mazeLine(hangman, 94, 53, 94, 127, value);
}

//*****************************************************************************
//
//! Depending on the game condition draws the win or lose screen
//! Also updates the high score with AWS if needed
//!
//! \param  result is a 1 for a win and 0 for a loss
//! \param  lRetVal is return value from the TLS connection
//!
//! \return None
//!
//*****************************************************************************

void winLoseScreen(int result,int lRetVal){
    fillScreen(BLACK);
    char scoreStr[10]; // Buffer to hold the score as a string
    sprintf(scoreStr, "%d", score); // Convert the integer score to a string

    if(result){
        if(score > currentHighScore){
            char *jsonString = generateJSON(scoreStr);
           if (jsonString != NULL) {
               http_post(lRetVal,jsonString);
               free(jsonString); // Free allocated memory
           } else {
               printf("Failed to generate JSON string\n");
           }
        }
        setTextColor(GREEN,BLACK);
        setCursor(10,15);
        setTextSize(2);
        Outstr("CONGRATS!!!");
        setCursor(10,45);
        Outstr("You WIN");
        setCursor(10,75);
        setTextSize(1);
        Outstr("Your Score: ");
        setCursor(75,75);
        Outstr(scoreStr);
    }
    else{
        setTextColor(RED,BLACK);
        setCursor(10,15);
        setTextSize(2);
        Outstr("YOU LOST!!!");
        setCursor(10,75);
        setTextSize(1);
        Outstr("Your Score: ");
        setCursor(75,75);
        Outstr(scoreStr);
    }
}
//*****************************************************************************
//
//  Menu Selection Functions - END
//
//*****************************************************************************

//*****************************************************************************
//
//  Hangman Punishment Functions
//
//*****************************************************************************
//*****************************************************************************
//
//! Draw the head of the hangman
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************

void DrawHead(){
    drawCircle(89, 26, 3,WHITE);
}

//*****************************************************************************
//
//! Draw the left arm of hangman
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************

void DrawLeftArm(){
    drawLine(89, 35, 80, 45,WHITE);
}

//*****************************************************************************
//
//! Draw the right arm of hangman
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************
void DrawRightArm(){
    drawLine(89, 35, 98, 45,WHITE);
}

//*****************************************************************************
//
//! Draw the left leg of hangman
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************

void DrawLeftLeg(){
    drawLine(89, 49, 80, 58,WHITE);
}

//*****************************************************************************
//
//! Draw the right leg of hangman
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************

void DrawRightLeg(){
    drawLine(89, 49, 98, 58,WHITE);
}

//*****************************************************************************
//
//! Draw the core of Hangman
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************

void DrawCore(){
    drawLine(89, 30, 89, 50,WHITE);
}

//*****************************************************************************
//
//  Hangman Punishment Functions - END
//
//*****************************************************************************

//*****************************************************************************
//
//  Accelerometer Functions
//
//*****************************************************************************

//*****************************************************************************
//
//! Display the buffer contents over I2C
//!
//! \param  pucDataBuf is the pointer to the data store to be displayed
//! \param  ucLen is the length of the data to be displayed
//!
//! \return none
//! This function was given in the demo
//*****************************************************************************
void
DisplayBuffer(unsigned char *pucDataBuf, unsigned char ucLen, unsigned char axis)
{
    unsigned char ucBufIndx = 0;
    if(axis == 0x03){
        UART_PRINT("X Axis Read contents");
    }
    else if(axis == 0x05){
        UART_PRINT("Y Axis Read contents");
    }
    else{
        UART_PRINT("Read contents");
    }
    UART_PRINT("\n\r");
    while(ucBufIndx < ucLen)
    {
        UART_PRINT(" 0x%x, ", pucDataBuf[ucBufIndx]);
        ucBufIndx++;
        if((ucBufIndx % 8) == 0)
        {
            UART_PRINT("\n\r");
        }
    }
    UART_PRINT("\n\r");
}

//*****************************************************************************
//
//! Read the Accelerometer
//!
//! \param  axis is the register offset for the x or y register
//!
//! \return aucRdDataBuf[0] which is our acceleration reading in two's complement
//!
//*****************************************************************************
unsigned char readAccelerometer(unsigned char axis)
{
    unsigned char ucDevAddr = ACCELEROMETER_ADDRESS; // Device address
    unsigned char ucRegOffset = axis; // Register offset
    unsigned char ucRdLen = 1; // Read length
    unsigned char aucRdDataBuf[256];

    // Write the register address to be read from.
    RET_IF_ERR(I2C_IF_Write(ucDevAddr, &ucRegOffset, 1, 0));

    // Read the specified length of data
    RET_IF_ERR(I2C_IF_Read(ucDevAddr, &aucRdDataBuf[0], ucRdLen));

    UART_PRINT("I2C Read From address complete\n\r");

    // Display the buffer over UART on successful readreg
    DisplayBuffer(aucRdDataBuf, ucRdLen,axis);

    // Return the first item in the read data buffer
    return aucRdDataBuf[0];
}

//*****************************************************************************
//
//! Convert a two's complement value into an integer
//!
//! \param  value is the two's complement value
//!
//! \return value, converted to an integer
//!
//*****************************************************************************
int twosComplementToInteger(unsigned char value) {
    // Check MSB
    if (value & 0x80) { // MSB is 1, value is negative
        // Invert bits
        value = ~value;
        // Add 1
        value += 1;
        // Convert to integer and assign negative sign
        return -(int)value;
    } else { // MSB is 0, value is positive
        return (int)value;
    }
}

//*****************************************************************************
//
//! Give an average of the last 5 angles
//!
//! \param  newValue is the new angle to be accounted for
//! \param  axis is the axis of the reading
//!
//! \return acceleration is the acceleration read from the accelerometer
//!
//*****************************************************************************

int smoothAcceleration(int newValue, int axis) {
    static int accelData[2][ACCEL_AVERAGE_WINDOW] = {0}; // One array per axis
    static int accelIndex[2] = {0}; // One index per axis
    static int accelSum[2] = {0}; // One sum per axis
    int average;

    // Update sum and data array
    accelSum[axis] -= accelData[axis][accelIndex[axis]];
    accelData[axis][accelIndex[axis]] = newValue;
    accelSum[axis] += newValue;

    // Update index
    accelIndex[axis] = (accelIndex[axis] + 1) % ACCEL_AVERAGE_WINDOW;

    // Calculate average
    average = accelSum[axis] / ACCEL_AVERAGE_WINDOW;

    return average;
}

//*****************************************************************************
//
//! Read the Accelerometer and convert the value to an integer
//!
//! \param  axis is the axis to be read from the accelerometer
//!
//! \return acceleration is the acceleration read from the accelerometer
//!
//*****************************************************************************
int getAcceleration(unsigned char axis){
//    int axis1 = 1;
//    if(XAXISREGISTEROFFSET){
//        axis1 = 0;
//    }
    // Retrieve the acceleration of the axis in two's complement
    unsigned char preprocessedAcceleration = readAccelerometer(axis);
    // Convert the acceleration into an integer
    int acceleration = twosComplementToInteger(preprocessedAcceleration);
    // Smooth the acceleration
    //int smoothedAcc = smoothAcceleration(acceleration,axis1);
    // Return our acceleration
    return acceleration;
}

//*****************************************************************************
//
//  Accelerometer Functions - END
//
//*****************************************************************************

//*****************************************************************************
//
//  Ball Movement Functions
//
//*****************************************************************************

//*****************************************************************************
//
//! Calculate the shift in direction from the given angular acceleration and current position
//!
//! \param  value is the current position
//! \param  angle is the angular acceleration
//! \param  axis is the axis of the pixel
//!
//! \return shiftedPixel is the pixel moved proportional to the acceleration
//!
//*****************************************************************************
int calculateShift(int value, int angle){
    int shiftedPixel;
    float rate = 0.1; //  pixels per angle

    // Check if speed exceeds the maximum allowed value
    if (angle > 90) {
        angle = 90; // Limit speed to maximum allowed value
    } else if (angle < -90) {
        angle = -90; // Limit speed to minimum allowed value
    }

    shiftedPixel = value - (int)(rate * angle);


    return shiftedPixel;
}

//*****************************************************************************
//
//! Checks if the new position is inside the bounds and also checks if the
//! new position is in a wall.
//!
//! \param  x is the updated position
//! \param  y is the updated position
//! \param  ballRadius is the size of the ball
//! \param  maze is the array of the maze
//!
//! \return 1 if the coordinates are valid and a 0 if they are invalid
//!
//*****************************************************************************
int isValidPosition(int x, int y, int ballRadius, char maze[SSD1351HEIGHT][SSD1351WIDTH]) {
    // Ensure the ball's coordinates are within the bounds of the maze
    if (x - ballRadius < 0 || x + ballRadius >= SSD1351WIDTH ||
        y - ballRadius < 0 || y + ballRadius >= SSD1351HEIGHT) {
        return 0; // Out of bounds
    }
    int i, j;
    // Check each point around the circumference of the ball
    for (i = -ballRadius; i <= ballRadius; i++) {
        for (j = -ballRadius; j <= ballRadius; j++) {
            if (i * i + j * j <= ballRadius * ballRadius) { // Check points within the circle
                int checkX = x + i;
                int checkY = y + j;

                // Ensure the check is within bounds (redundant due to previous check but safe)
                if (checkX < 0 || checkX >= SSD1351WIDTH ||
                    checkY < 0 || checkY >= SSD1351HEIGHT) {
                    return 0; // Out of bounds
                }

                // Check if any point on the circumference is a wall
                if (maze[checkY][checkX] == 'x') {
                    return 0; // Invalid position
                }
            }
        }
    }

    return 1; // Valid position
}


//*****************************************************************************
//
//  Ball Movement Functions - END
//
//*****************************************************************************

//*****************************************************************************
//
//  Game Functions
//
//*****************************************************************************
//*****************************************************************************
//
//! Run the Maze Game
//!
//! \param  maze is the selected maze
//! \param  startingX is the x coordinate starting position
//! \param  startingY is the y coordinate starting position
//! \param  ballRadius is the size of the ball to be controlled
//!
//! \return None
//!
//*****************************************************************************
void mazeGame(char maze[SSD1351HEIGHT][SSD1351WIDTH], int startingX, int startingY, int endX, int endY, int ballRadius) {
    int x = startingX;
    int y = startingY;
    int r = ballRadius;
    int goalRadius = 2; // Radius of the goal circle

    int collisions = 0;

    fillCircle(startingX, startingY, goalRadius, GREEN);
    fillCircle(endX, endY, goalRadius, RED);
    drawCircle(x, y, r, CYAN);

    while (1) {
        if(checkFlag && dataStream == DIGITONE){
            checkFlag = 0;
            gameResult = 0;
            score = 0;
            currentState = WIN_LOSE_SCREEN;
            previousState = MAZE_STATE;
            break;
        }
        fillCircle(startingX, startingY, goalRadius, GREEN);
        fillCircle(endX, endY, goalRadius, RED);

        int roll = getAcceleration(XAXISREGISTEROFFSET);
        int pitch = getAcceleration(YAXISREGISTEROFFSET);

        int oldX = x;
        int oldY = y;

        int newX = calculateShift(x, roll); // X axis is 0
        int newY = calculateShift(y, pitch); // Y axis is 1

        // Check if the new position is valid
        if (isValidPosition(newX, newY, r, maze)) {
            x = newX;
            y = newY;
        }
        else{
            collisions++;
        }

        drawCircle(oldX, oldY, r, BLACK);
        drawCircle(x, y, r, CYAN);

        // Calculate distance to the goal
        int dx = x - endX;
        int dy = y - endY;
        int distance = sqrt(dx * dx + dy * dy);

        // Check if the ball is within the goal area
        if (distance <= r + goalRadius) {
            gameResult = 1;
            score = score - collisions;
            previousState = MAZE_STATE;
            currentState = WIN_LOSE_SCREEN;
            break;
        }
    }
}

//*****************************************************************************
//
//! Run the hangman Game
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************
void hangmanGame(){

    int numOfMistakes = 0;
    char word[5] = "world";
    char currLetter;
    int i;


    while (1) {
        //Wait for letter from remote
        //currLetter = letter from remote controller;
        if(checkFlag && dataStream == DIGITONE){
            checkFlag = 0;
            gameResult = 0;
            score = 0;
            currentState = WIN_LOSE_SCREEN;
            previousState = HANGMAN_STATE;
            break;
        }

        for ( i = 0; i < 5; i++){
            if (word[i] == currLetter){
                word[i] = '0';
                setCursor(cursorsArray[i][0],cursorsArray[i][1]);
                Outstr(currLetter);
                continue;
            }
        }
        // Letter was not found

        numOfMistakes++;

        switch (numOfMistakes)
        {
        case 1:
            DrawHead();
            break;
        case 2:
            DrawCore();
            break;
        case 3:
            DrawLeftArm();
            break;
        case 4:
            DrawRightArm();
            break;
        case 5:
            DrawLeftLeg();
            break;
        case 6:
            DrawRightLeg();
            // Report end of game, exit the function
            break;
        default:
            break;
        }


    }
}

//*****************************************************************************
//
//  Game Functions - END
//
//*****************************************************************************
//*****************************************************************************
//
//  Initialization Functions
//
//*****************************************************************************

//*****************************************************************************
//
//! SPI Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void SPIInit(){
    //
    // Enable the SPI module clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);

    //
    // Reset the peripheral
    //
    MAP_PRCMPeripheralReset(PRCM_GSPI);

    //
    // Reset SPI
    //
    MAP_SPIReset(GSPI_BASE);

    //
    // Configure SPI interface
    //
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));

    //
    // Enable SPI for communication
    //
    MAP_SPIEnable(GSPI_BASE);
}

//*****************************************************************************
//
//! SPI Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************

static void OLEDInit(void){
    //
    // Initialize the OLED
    //
    Adafruit_Init();

    //
    //Initialize the screen
    //
    fillScreen(BLACK);
}

//*****************************************************************************
//
//! SYSTICK Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void SysTickInit(void) {

    // configure the reset value for the systick countdown register
    MAP_SysTickPeriodSet(SYSTICK_RELOAD_VAL);

    // register interrupts on the systick module
    MAP_SysTickIntRegister(SysTickHandler);

    // enable interrupts on systick
    // (trigger SysTickHandler when countdown reaches 0)
    MAP_SysTickIntEnable();

    // enable the systick module itself
    MAP_SysTickEnable();
}

//*****************************************************************************
//
//! IR Remote Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void IRRemInit(void){

    MAP_GPIOIntRegister(IRpin.port, IRIntHandler);                // Register the INterrupt Handler
    MAP_GPIOIntTypeSet(IRpin.port, IRpin.pin, GPIO_FALLING_EDGE); // configure falling edge interrupts from the IR
    uint64_t ulStatus = MAP_GPIOIntStatus(IRpin.port, false);
    MAP_GPIOIntClear(IRpin.port, ulStatus);// clear interrupts on the IR
    // Enable interrupts on IR
    MAP_GPIOIntEnable(IRpin.port, IRpin.pin);
}
//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//*****************************************************************************
//
//! Main function handling the Game Bundle
//!
//! \param  None
//!
//! \return None
//! 
//*****************************************************************************
void main()
{
    long lRetVal = -1;
    //
    // Initialize board configurations
    //
    BoardInit();
    
    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();
    
    //
    // Configuring UART
    //
    InitTerm();
    ClearTerm();
    
    //
    // Configure SPI
    //
    SPIInit();

    //
    // I2C Init
    //
    I2C_IF_Open(I2C_MASTER_MODE_FST);

    //
    // Enable SysTick
    //
    SysTickInit();

    //
    // Enable IR interrupts
    //
    IRRemInit();
    IR_intflag = 0;
    IR_intcount = 0;

    //
    // Initialize the OLED
    //
    OLEDInit();

    //
    // initialize global default app configuration
    //
    g_app_config.host = SERVER_NAME;
    g_app_config.port = GOOGLE_DST_PORT;

    //
    //Connect the CC3200 to the local access point
    //
    lRetVal = connectToAccessPoint();

    //
    //Set time so that encryption can be used
    //
    lRetVal = set_time();
    if(lRetVal < 0) {
        UART_PRINT("Unable to set time in the device");
        LOOP_FOREVER();
    }

    //
    //Connect to the website with TLS encryption
    //
    lRetVal = tls_connect();
    if(lRetVal < 0) {
        ERR_PRINT(lRetVal);
    }

    currentHighScore = http_get(lRetVal);   // initialize the high score

    buildHangman('x');      // pre-load Hangman

    currentState = MAIN_MENU_STATE;
    previousState = MAIN_MENU_STATE;

    while(1){
        if(IR_intflag){             // check if an interrpt occured
            IR_intflag = 0;
            if (currentState == CURSOR_STATE && checkFlag){             // dataStream is ready to be checked
                if (previousState == MAIN_MENU_STATE){              // Next statements check the current and previous state for the action to take based on the IR remote interrupt
                    if(dataStream == DIGITMUTE && checkFlag){
                        checkFlag = 0 ;
                        if (currentCursorLocation == 0){
                            checkFlag = 0;
                            currentState = MAZE_DIFFICULTY_SCREEN;
                            previousState = CURSOR_STATE;
                        }
                        else if (currentCursorLocation == 2){
                            checkFlag = 0;
                            currentState = HIGHSCORE_STATE;
                            previousState = CURSOR_STATE;
                        }
                        else if (currentCursorLocation == 1){
                            checkFlag = 0;
                            currentState = HANGMAN_SCREEN;
                            previousState = CURSOR_STATE;
                        }
                        else{
                            printf("Not Built Yet");
                        }
                    }
                    else if((dataStream == DIGITTWO || dataStream == DIGITFIVE) && checkFlag){
                        checkFlag = 0;
                        changeCursorLocation(mainCursorArray,2);
                    }
                    else{
                        checkFlag = 0;
                        printf("Not Recognized");
                    }
                }
                else if (previousState == MAZE_DIFFICULTY_SCREEN){
                    if((dataStream == DIGITTWO || dataStream == DIGITFIVE) && checkFlag){
                        checkFlag = 0;
                        changeCursorLocation(mazeDifCursorArray,2);
                    }
                    else if (dataStream == DIGITMUTE){
                        checkFlag = 0 ;
                        currentState = MAZE_STATE;
                        previousState = CURSOR_STATE;
                    }
                }
                else if (previousState == HIGHSCORE_STATE){
                    if(dataStream == DIGITMUTE){
                        checkFlag = 0 ;
                        currentState = MAIN_MENU_STATE;
                        previousState = CURSOR_STATE;
                    }

                }
                else if (previousState == WIN_LOSE_SCREEN){
                    if(dataStream == DIGITMUTE){
                        checkFlag = 0 ;
                        currentState = MAIN_MENU_STATE;
                        previousState = WIN_LOSE_SCREEN;
                    }

                }
                else if (previousState == HANGMAN_SCREEN){
                    if(dataStream == DIGITONE){
                        checkFlag = 0 ;
                        currentState = MAIN_MENU_STATE;
                        previousState = CURSOR_STATE;
                    }

                }
            }
        }
        else{
            switch (currentState){          // This is the state machine for switching between screens
                case MAIN_MENU_STATE:
                    mainMenu();
                    gameResult = 0;
                    dataStream = 0;
                    checkFlag = 0;
                    currentCursorLocation = 0;
                    currentCursor.xCoor = MAZECURSORX;
                    currentCursor.yCoor = MAZECURSORY;
                    currentState = CURSOR_STATE;
                    previousState = MAIN_MENU_STATE;
                    break;

                case CURSOR_STATE:
                    checkFlag = 0;
                    if(previousState != HIGHSCORE_STATE && previousState != WIN_LOSE_SCREEN && previousState != HANGMAN_SCREEN){
                        blinkCursor(currentCursor.xCoor,currentCursor.yCoor,WHITE);
                    }
                    break;

                case MAZE_DIFFICULTY_SCREEN:
                    mazeDifficultySelect();
                    checkFlag = 0;
                    currentCursor.xCoor = EASYCURSORX;
                    currentCursor.yCoor = EASYCURSORY;
                    currentState = CURSOR_STATE;
                    previousState = MAZE_DIFFICULTY_SCREEN;
                    break;

                case MAZE_STATE:
                    previousState = CURSOR_STATE;
                    int mazeStartX, mazeStartY, mazeEndX,mazeEndY;
                    checkFlag = 0;
                    if(currentCursorLocation == 0){
                        mazeStartX = 9;
                        mazeStartY = 122;
                        mazeEndX = 121;
                        mazeEndY = 7;
                        buildEasyMaze(genericMaze,'x');
                        drawMaze(genericMaze);
                        mazeGame(genericMaze,mazeStartX,mazeStartY,mazeEndX,mazeEndY,1);
                    }
                    else if(currentCursorLocation == 1){

                        mazeStartX = 122;
                        mazeStartY = 40;
                        mazeEndX = 122;
                        mazeEndY = 98;
                        buildMediumMaze(genericMaze,'x');
                        drawMaze(genericMaze);
                        mazeGame(genericMaze,mazeStartX,mazeStartY,mazeEndX,mazeEndY,1);
                    }
                    else if(currentCursorLocation == 2){
                        mazeStartX = 59;
                        mazeStartY = 6;
                        mazeEndX = 7;
                        mazeEndY = 43;
                        buildHardMaze(genericMaze,'x');
                        drawMaze(genericMaze);
                        mazeGame(genericMaze,mazeStartX,mazeStartY,mazeEndX,mazeEndY,1);
                    }
                    else{
                        printf("Invalid Cursor Location");
                        break;
                    }

                    break;

                case HIGHSCORE_STATE:
                    highScoresScreen();
                    checkFlag = 0 ;
                    currentHighScore = http_get(lRetVal);
                    char scoreStr[10]; // Buffer to hold the score as a string
                    int tempscore = 754;
                    sprintf(scoreStr, "%d", tempscore); // Convert the integer score to a string
                    setTextSize(1);
                    setCursor(15,31);
                    setTextColor(WHITE,BLACK);
                    Outstr(scoreStr);
                    currentState = CURSOR_STATE;
                    previousState = HIGHSCORE_STATE;
                    break;
                case HANGMAN_SCREEN:
                    drawMaze(hangman);

                    currentState = HANGMAN_STATE;
                    previousState = HANGMAN_SCREEN;
                    break;
                case HANGMAN_STATE:
                    hangmanGame();
                    break;
                case WIN_LOSE_SCREEN:
                    clearMaze(genericMaze);
                    checkFlag = 0;
                    winLoseScreen(gameResult,lRetVal);
                    previousState = WIN_LOSE_SCREEN;
                    currentState = CURSOR_STATE;

                default:
                    printf("Unknown state: %d\n", currentState);
                    break;
            }
        }
    }

}

//*****************************************************************************
//
// Close the Doxygen group.
//! @
//
//*****************************************************************************


