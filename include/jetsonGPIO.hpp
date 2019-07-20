/*
 * jetsonGPIO.h
 *
 * Copyright (c) 2015 JetsonHacks
 * www.jetsonhacks.com
 *
 * Based on Software by RidgeRun
 * Originally from:
 * https://developer.ridgerun.com/wiki/index.php/Gpio-int-test.c
 * Copyright (c) 2011, RidgeRun
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the RidgeRun.
 * 4. Neither the name of the RidgeRun nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY RIDGERUN ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL RIDGERUN BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef JETSONGPIO_H_
#define JETSONGPIO_H_

/****************************************************************
 * Constants
 ****************************************************************/

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

typedef unsigned int jetsonGPIO;
typedef unsigned int pinDirection;
typedef unsigned int pinValue;

enum pinDirections { inputPin = 0, outputPin = 1 };

enum pinValues {
  low = 0,
  high = 1,
  off = 0, // synonym for things like lights
  on = 1
};

// jetsonTK1GPIO??
// enum jetsonGPIONumber {
//  gpio57 = 57,   // J3A1 - Pin 50
//  gpio160 = 160, // J3A2 - Pin 40
//  gpio161 = 161, // J3A2 - Pin 43
//  gpio162 = 162, // J3A2 - Pin 46
//  gpio163 = 163, // J3A2 - Pin 49
//  gpio164 = 164, // J3A2 - Pin 52
//  gpio165 = 165, // J3A2 - Pin 55
//  gpio166 = 166  // J3A2 - Pin 58
//};

// jetsonTX1GPIO
// enum jetsonTX1GPIONumber {
//  gpio36 = 36,   // J21 - Pin 32 - Unused - AO_DMIC_IN_CLK
//  gpio37 = 37,   // J21 - Pin 16 - Unused - AO_DMIC_IN_DAT
//  gpio38 = 38,   // J21 - Pin 13 - Bidir  - GPIO20/AUD_INT
//  gpio63 = 63,   // J21 - Pin 33 - Bidir  - GPIO11_AP_WAKE_BT
//  gpio184 = 184, // J21 - Pin 18 - Input  - GPIO16_MDM_WAKE_AP
//  gpio186 = 186, // J21 - Pin 31 - Input  - GPIO9_MOTION_INT
//  gpio187 = 187, // J21 - Pin 37 - Output - GPIO8_ALS_PROX_INT
//  gpio219 = 219, // J21 - Pin 29 - Output - GPIO19_AUD_RST
//
//  //  自查都可用？？
//  //  gpio162 = 162, // J21 - Pin11 - GPIO_GEN0 - UART #0 Request to Send
//  //  gpio11 = 11,   // J21 - Pin12 - GPIO_GEN1 - Audio I2S #0 Clock
//  //  gpio38 = 38,   // J21 - Pin13 - GPIO_GEN2 - Bidir  - Audio Code
//  Interrupt
//  //  gpio511 = 511, // J21 - Pin15 - GPIO_GEN3 - From GPIO Expander (P17)
//  //  gpio37 = 37,   // J21 - Pin16 - GPIO_GEN4 - Unused
//  //  gpio184 = 184, // J21 - Pin18 - GPIO_GEN5 - Input  - Modem Wake AP GPIO
//  //  gpio510 = 510, // J21 - Pin22 - GPIO_GEN6 - From GPIO Epander (P16)
//  //
//  //  gpio219 = 219, // J21 - Pin29 - GPIO5     - Output - Audio Reset
//  //  (1.8/3.3V) gpio186 = 186, // J21 - Pin31 - GPIO6     - Input  - Motion
//  //  Interrupt (3.3V) gpio36 = 36,   // J21 - Pin32 - GPIO12    - Unused
//  gpio63
//  //  = 63,   // J21 - Pin33 - GPIO13    - Bidir  - AP Wake Bt GPIO gpio163 =
//  //  163, // J21 - Pin36 - GPIO16    - UART #0 Clear to Send gpio8 = 8, //
//  //  J21 - Pin35 - GPIO19    - Audio I2S #0 Left/Right Clock gpio9 = 9, //
//  //  J21 - Pin38 - GPIO20    - Audio I2S #0 Data in gpio10 = 10,   // J21 -
//  //  Pin40 - GPIO21    - Audio I2S #0 Data in gpio187 = 187, // J21 - Pin37 -
//  //  GPIO26    - Output - (3.3V)
//};

// jetsonTX2GPIO
enum jetsonTX2GPIONumber {
  gpio397 = 397, // J21 - Pin 13  - GPIO_GEN2 Audio Code Interrupt
  gpio389 = 389, // J21 - Pin 33  - GPIO13 AP Wake Bt GPIO
  gpio481 = 481, // J21 - Pin 18  - GPIO_GEN5 Modem Wake AP GPIO
  gpio398 = 398, // J21 - Pin 29  - GPIO5 Audio Reset (1.8/3.3V)

};

int gpioExport(jetsonGPIO gpio);
int gpioUnexport(jetsonGPIO gpio);
int gpioSetDirection(jetsonGPIO, pinDirection out_flag);
int gpioSetValue(jetsonGPIO gpio, pinValue value);
int gpioGetValue(jetsonGPIO gpio, unsigned int *value);
int gpioSetEdge(jetsonGPIO gpio, char *edge);
int gpioOpen(jetsonGPIO gpio);
int gpioClose(int fileDescriptor);
int gpioActiveLow(jetsonGPIO gpio, unsigned int value);

#endif // JETSONGPIO_H_
