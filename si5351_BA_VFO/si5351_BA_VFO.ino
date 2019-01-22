/*! \mainpage si5351 Boatanchor 3-band VFO
 *
 * This sketch implements a three band VFO using the SI5351a 
 * breakout board from Etherkit. 
 *  Features include:
 *  - Operation on a single frequency at a time, push button selected
 *  - Frequency adjustment by rotary encoder. Rate of adjustment is 
 *    push button adjustable in 10 to 10000 Hz increments.
 *  - Display shows currently selected VFO, VFO status, VFO 
 *    frequency. The display header is customizable, and shows 
 *    current frequency increment.
 *  - Choose your favorite band and frequencies as default start up
 *    values.
 */
 
/**
 * @file
 * @author  Mike Aiello N2HTT <n2htt@arrl.net>
 * @version 1.4
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This sketch implements a three band VFO using the SI5351a 
 * breakout board from Etherkit. It should run on any AT328 
 * Arduino. Required resources are:
 *  - 32k program memory
 *  - 1k variable space
 *  - 2 pins supporting external hardware interrupts
 *  - 3 digital input pins
 *  - I2C bus support for SI5351 and a 20x4 LCD display with I2C adapter
 *
 * The prototype was tested on:
 *  - Arduino Mega 2560 & Uno R3
 *  - Etherkit SI5351a breakout board
 *  - Bournes mechanical encoder, 24 ppr with momentary contact switch
 *  - SPST momentary pushbutton switch
 *  - 20x4 line LCD with I2C adapter
 *
 * The macros immediately following this description configure all the 
 * hardware options for the VFO. Configurable items include:
 *  - Hardware pin assignments
 *  - VFO frequency settings
 *  - Number of VFOs
 *  - Whether to display a heading line
 *  - Range of frequency increments
 *  - Other timing parameters
 *
 * See the include files for other configurable parameters.
 *
 * Libraries used in this sketch are:
 *  - Francisco Malpartida  
 *    New LiquidCrystal- interface for the 20x4 LCD display with I2C. This is 
 *                       not supplied, but is available here:
 *                       https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
 *
 *  - Etherkit SI5351  - interface for the SI5351a clock chip, 
 *                       not supplied but available here:
 *                       https://github.com/etherkit/Si5351Arduino
 *
 * Object wrappers are supplied for the display, and 
 * the SI5351. Each wrapper is constructed with a generic 
 * abstract base class that defines the methods used by the sketch, 
 * and then a hardware specific class that uses the appropriate third 
 * party library to implement the methods.
 * This architecture will make it easier to adapt the sketch to use
 * other harware or libraries in the future. The wrappers classes are:
 *
 * Base class       Implementation class(es)
 * VFODefinition    si5351_VFODefinition
 * VFODisplay       LCD2004_LCDLib_VFODisplay
 *
 * Some notes about program structure:
 * 1. I would have normally included the hardware specific library
 *    includes in the include files containing the hardware specific
 *    classes, but the Arduino IDE did not find them when I did. For
 *    that reason you will see them below in this file, above the local 
 *    includes for hardware specific classes.
 * 2. The include files FrequencySelection.h and DeviceInitializations.h
 *    contain sections of in-line c++ code that I moved to separate
 *    files for better readability. Using the include mechanism allows
 *    inserting this code in the main sketch in the proper sequence.
 * 3. The embedded comment blocks in all of the files of this sketch
 *    are compatible with documentation generators like javadoc
 *    and doxygen.
 */
 
/**
 * Uncomment the two lines below to use the LiquidCrystal library
 * with the 20 x 4 LCD display. If these are uncommented, be sure 
 * that the lines for the SSD106 display are commented out
 */
#define USE_LIQUIDCRYSTAL_LIBRARY
#define USE_LCD_20X4_DISPLAY 

/**
 * Uncomment the line below to use the initialization parameters
 * for the Sainsmart 20 x 4 LCD display with I2C backpack installed. 
 * Leave it commented out if you are using an add-on I2C backpack.
 */
 //  #define USE_SAINSMART_LCD_WITH_I2C_BACKPACK

#include <Wire.h>
#include <SPI.h>

/**
 * Uncomment the line below to use the old version 1.x si5351
 * library. 
 * Leave it commented out if you are using the current version, 2.x.
 */
 // #define USE_VERSION_1_SI5351_LIBRARY

#include <si5351.h>

#ifdef USE_LIQUIDCRYSTAL_LIBRARY
#include <LiquidCrystal_I2C.h>
#endif

/**
 * Pared-down implementation of DigitalPin/DigitalPulse
 * used as a non-library, local implementation of the 
 * DigitalPin and DigitalPulse classes.
 * 
 * The local versions of these classes have all unused 
 * functionality removed, yielding the minimum flash memory
 * size.
 */
#include "SimpleDigitalInputPin.h"
#include "SimpleDigitalPulse.h"
#define INPUT_PIN_TYPE SimpleDigitalInputPin

#include "si5351_VFODefinition.h"

#ifdef USE_LIQUIDCRYSTAL_LIBRARY
   #ifdef USE_LCD_20X4_DISPLAY
      #include "LCD2004_LCDLIB_VFODisplay.h"
   #endif
#endif

/**
 * The following two lines control whether a heading
 * line is shown on the display. Uncomment the 
 * setting you want. The no heading display is
 * intended primarily for 2 line LCD displays
 */
//#define DISPLAY_HEADER_LINE      NO_HEADING
#define DISPLAY_HEADER_LINE    SHOW_HEADING

/**
 * hardware pin definitions
 * definitions for UNO R3
 */
#define ENCODER_PIN_A                     2
#define ENCODER_PIN_B                     3
#define VFO_SELECTOR_PIN                  4
#define FRQ_DELTA_SELECTOR_PIN            5
#define VFO_KEYED_PIN                     6

/**
 * frequency change constants
 */
#define FREQ_DELTA_DEFAULT              100
#define FREQ_DELTA_MAX                10000
#define FREQ_DELTA_MIN                   10
#define FREQ_DELTA_MULT                  10

/**
 * The following two lines control how many 
 * VFOs the sketch will control - maximum 3. 
 * You can reduce the number of VFOs if you are
 * using a display with fewer than 3 lines
 */
#define NUMBER_OF_VFOS                    3
#define STARTING_VFO                      1

/**
 * vfo frequency limits
 * set your frequency ranges
 * and starting frequency for each band here
 */
#define FREQ_VFO_A_DEFAULT          3560000
#define FREQ_VFO_A_MIN              3500000
#define FREQ_VFO_A_MAX              4000000

#define FREQ_VFO_B_DEFAULT          7055000
#define FREQ_VFO_B_MIN              7000000
#define FREQ_VFO_B_MAX              7300000

#define FREQ_VFO_C_DEFAULT         14060000
#define FREQ_VFO_C_MIN             14000000
#define FREQ_VFO_C_MAX             14350000

/**
 * delay times
 */
#define BUTTON_DEBOUNCE_WAIT_MILS         5
#define SETUP_DELAY_MILS                  5
#define LOOP_DELAY_MILS                   1
#define FREQ_DELTA_LATENCY_MILS         900

/**
 * Si5351 clock board object
 */
Si5351 si5351;

/**
 * vfo list and variables controlling frequency
 */
VFODefinition *vfoList[NUMBER_OF_VFOS];

short currVFO;
VFODefinition *pCurrentVFO;
short freq_delta_display_time;
unsigned long frequency_delta;

/**
 * fixed frequency offset
 * this is normally set to zero, but can be set to a 
 * positive or negative frequency offset (in Hertz)
 * that will be applied to every frequency set operation
 *
 * This offset does not appear in the display value.
 * For example, if setting the VFO to 14000000 Hz (14 MHz)
 * consistently results in a measured frequency of 
 * 14000135 Hz using an external counter, you can
 * set fixedFrequencyOffset = -135LL to correct the 
 * measured frequency to match the displayed frequency
 *
 */
long long VFODefinition::fixedFrequencyOffset = 0LL;

/**
 * display object
 */
VFODisplay *pDisplay;

/**
 * digital pins (reading button presses)
 */ 
INPUT_PIN_TYPE VFOSelectPin( VFO_SELECTOR_PIN
                                , INPUT_PULLUP
                                , BUTTON_DEBOUNCE_WAIT_MILS
                                , DIGITAL_PIN_INIT_STATE_HIGH
                                , DIGITAL_PIN_INVERTING);

INPUT_PIN_TYPE FrequencyDeltaSelectPin( FRQ_DELTA_SELECTOR_PIN
                                       , INPUT_PULLUP
                                       , BUTTON_DEBOUNCE_WAIT_MILS
                                       , DIGITAL_PIN_INIT_STATE_HIGH
                                       , DIGITAL_PIN_INVERTING);


INPUT_PIN_TYPE VFOKeyedPin( VFO_KEYED_PIN
                          , INPUT_PULLUP
                          , 0 // this pin is not software debounced
                          , DIGITAL_PIN_INIT_STATE_HIGH
                          , DIGITAL_PIN_INVERTING);


/**
 * handler code for frequency selection via rotary encoder
 */
#include "FrequencySelection.h"

/**
 * initialization code for hardware devices
 */
#include "DeviceInitializations.h"

/**
 * sketch setup
 */
void setup()   { 
   // initialize serial port at a relatively languid rate               
   //Serial.begin(9600);
   
   // initialize digital input pins for button presses               
   setupInputPins();
   
   // setup encoder for frequency adjustments               
   setupEncoder();
   
   // initialize vfo list                
   setupVFOs();  
   
   // initialize VFO clocks (all off)               
   initializeVFOs();   
   
   // light up display               
   setupDisplay(); 
   
   delay(SETUP_DELAY_MILS);
}

/**
 * operating loop
 */
void loop() {
   // selecting output vfo is first priority
   if (VFOSelectPin.readInputPulseMode()) {
      int mode = VFOSelectPin.getCurrentPinMode();
      //Serial.println(mode);
      switch (mode) {
         case PIN_MODE_SHORT_PULSE:
            // short pulse - switch to next vfoList
            // - turn off current clock
            pCurrentVFO->stop();
            
            // increment the selected vfoList
            currVFO = (++currVFO) % NUMBER_OF_VFOS; 
            pCurrentVFO = vfoList[currVFO]; 
            
            // turn on new clock if not disabled
            pCurrentVFO->loadFrequency();
 
            // repaint the vfoList display
            pDisplay->showVFOs(frequency_delta, currVFO);

            // reset pin
            VFOSelectPin.setCurrentPinMode(PIN_MODE_IDLE);
            break;

         case PIN_MODE_LONG_PULSE:
            // long pulse - not used
			// you can program some other behavior here
			// triggered by holding the button for > 1 sec
            break;
      }
   }

   // then we can check to see if frequency delta needs to change
   if (FrequencyDeltaSelectPin.readInputPulseMode()) {
      int mode = FrequencyDeltaSelectPin.getCurrentPinMode();
      switch (mode) {
         case PIN_MODE_SHORT_PULSE:
            // short pulse - change frequency delta
            if (frequency_delta < FREQ_DELTA_MAX) {
               frequency_delta *= FREQ_DELTA_MULT;
            }
            else {
               frequency_delta = FREQ_DELTA_MIN;
            }

            // take over display to show new frequency delta
            pDisplay->showFreqDeltaDisplay(frequency_delta);
            freq_delta_display_time = FREQ_DELTA_LATENCY_MILS;

            // reset pin
            FrequencyDeltaSelectPin.setCurrentPinMode(PIN_MODE_IDLE);
            break;

         case PIN_MODE_LONG_PULSE:
            // long pulse - not used
			// you can program some other behavior here
			// triggered by holding the button for > 1 sec
            break;
      }
   }
  
   // check to see if freq delta display needs to be overwritten
   if (freq_delta_display_time > 0) {
      // update the display time for the vfoList select pin
      freq_delta_display_time = (freq_delta_display_time > LOOP_DELAY_MILS)
                              ? (freq_delta_display_time -= LOOP_DELAY_MILS)
                              : 0;

      // repaint the vfoList display if freq display time exhausted
      if (freq_delta_display_time <= 0) {
         pDisplay->showVFOs(frequency_delta, currVFO);
      }
   }
  
   // re-display frequencies if encoder has made changes
   if (updateSelectedFrequencyValue()) {
      pDisplay->showVFOs(frequency_delta, currVFO); 
   }
   
   // let key line control clock operation
   VFOKeyedPin.determinePinState();
   if (VFOKeyedPin.getLogicalState()==HIGH) {
      // - turn on current clock
      pCurrentVFO->start();
   }
   else {
      // - turn off current clock
      pCurrentVFO->stop();
   }
   
   delay(LOOP_DELAY_MILS);
}

