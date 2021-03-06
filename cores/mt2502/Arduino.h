/*
  Arduino.h - Main include file for the Arduino SDK
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 20 Aug 2014 by MediaTek Inc.

*/

#ifndef Arduino_h
#define Arduino_h

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// some libraries and sketches depend on this
// AVR stuff, assuming Arduino.h or WProgram.h
// automatically includes it...
#include <avr/pgmspace.h>

#include "binary.h"
#include "itoa.h"
#include "vmdcl.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "wiring_constants.h"
#include <chip.h>

  void yield(void);

  /* sketch */
  extern void setup(void);
  extern void loop(void);
  extern void create(void);
  extern void paint(void);

  extern boolean changePinType(uint32_t ulPin, uint32_t ulPinType, VM_DCL_HANDLE *handle);
  extern void spiPinsRest(void);
  extern void setPinHandle(uint32_t ulPin, VM_DCL_HANDLE handle);

  typedef enum _EExt_Interrupts
  {
    EXTERNAL_INT_0 = 0, // A1
    EXTERNAL_INT_1 = 1, // A1
    EXTERNAL_INT_2 = 2, // A2
    EXTERNAL_INT_3 = 3, // E0
    EXTERNAL_INT_4 = 4, // E1
    EXTERNAL_INT_5 = 5, // E2
    EXTERNAL_INT_6 = 6, // E3
    EXTERNAL_INT_7 = 7, // E4
    EXTERNAL_NUM_INTERRUPTS
  } EExt_Interrupts;

  typedef void (*voidFuncPtr)(void);

#define PIO_MAX_NUM GPIO_MAX

  typedef enum _EPioType
  {
    PIO_DIGITAL,
    PIO_ANALOG,
    PIO_EINT,
    PIO_PWM,
    PIO_SPI,
    PIO_UART,
    PIO_I2C,
    PIO_SD,
    PIO_END
  } EPioType;

  /* Types used for the tables below */
  typedef struct _PinDescription
  {
    VM_DCL_HANDLE ulHandle;
    uint32_t ulGpioId;
    EPioType ulPinType;
    uint32_t ulPupd;
  } PinDescription;

  /* Pins table to be instanciated into variant.cpp */
  extern PinDescription g_APinDescription[];

#ifdef __cplusplus
} // extern "C"

#include "HardwareSerial.h"
#include "Tone.h"
#include "WCharacter.h"
#include "WMath.h"
#include "WString.h"
#include "wiring_pulse.h"

#endif // __cplusplus

// Include board variant
#include "variant.h"

#include "WInterrupts.h"
#include "wiring.h"
#include "wiring_analog.h"
#include "wiring_digital.h"
#include "wiring_shift.h"

#include "message.h"

#define USB_PID_LEONARDO 0x0034
#define USB_PID_MICRO 0x0035
#define USB_PID_DUE 0x003E

boolean noStopInterrupts(void);
extern void spiPinsRest(void);
extern void setPinHandle(uint32_t ulPin, VM_DCL_HANDLE handle);

#endif // Arduino_h
