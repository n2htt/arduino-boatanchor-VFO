#ifndef DEVICEINITIALIZATIONS_H
#define DEVICEINITIALIZATIONS_H

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
 * This file contains functions used to initialize hardware values.
 * 
 * Note that the SI5351 output current is hardcoded below at 2ma. You
 * can set it to higher values up to 8ma, see the documentation 
 * accompanying the Etherkit library.
 */
 
 
#include <Arduino.h> 

/**
 * initialize vfo list values
 */
void setupVFOs()   { 
#ifdef USE_VERSION_1_SI5351_LIBRARY    
   si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0);
#else   
   si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
#endif	
   
   delay(1); // let board settle after init
   
   vfoList[0] = new si5351_VFODefinition(si5351
                                   , FREQ_VFO_A_DEFAULT
                                   , FREQ_VFO_A_MIN
                                   , FREQ_VFO_A_MAX
                                   , SI5351_CLK0 
                                   , SI5351_PLLA
                                   , SI5351_DRIVE_2MA
                                   , true);
   
   vfoList[1] = new si5351_VFODefinition(si5351
                                   , FREQ_VFO_B_DEFAULT
                                   , FREQ_VFO_B_MIN
                                   , FREQ_VFO_B_MAX
                                   , SI5351_CLK0 
                                   , SI5351_PLLA
                                   , SI5351_DRIVE_2MA
                                   , true);
   
   vfoList[2] = new si5351_VFODefinition(si5351
                                   , FREQ_VFO_C_DEFAULT
                                   , FREQ_VFO_C_MIN
                                   , FREQ_VFO_C_MAX
                                   , SI5351_CLK0 
                                   , SI5351_PLLA
                                   , SI5351_DRIVE_2MA
                                   , true);

   frequency_delta          = FREQ_DELTA_DEFAULT;
   currVFO                  = STARTING_VFO;
   pCurrentVFO              = vfoList[currVFO]; 
   freq_delta_display_time  = 0;
}

/**
 * setup for digital pin input (button presses)
 */
void setupInputPins()   {  
   VFOSelectPin.initialize();  
   FrequencyDeltaSelectPin.initialize();
   VFOKeyedPin.initialize();   
}

/**
 * setup for rotary encoder (change frequencies)
 */
void setupEncoder()   {                
   // set up encoder pins for interrupts
   pinMode(ENCODER_PIN_A, INPUT); 
   pinMode(ENCODER_PIN_B, INPUT); 
   digitalWrite(ENCODER_PIN_A, HIGH);  // turn on pullup resistor
   digitalWrite(ENCODER_PIN_B, HIGH);  // turn on pullup resistor

   // encoder pin on interrupt 0 (pin 2)
   attachInterrupt(0, encoderPinA_ISR, CHANGE);

   // encoder pin on interrupt 1 (pin 3)
   attachInterrupt(1, encoderPinB_ISR, CHANGE);
}

/**
 * setup for OLED display
 */
void setupDisplay()   {   
#ifdef USE_LIQUIDCRYSTAL_LIBRARY
   #ifdef USE_LCD_20X4_DISPLAY
      pDisplay = new LCD2004_LCDLib_VFODisplay(vfoList
                                             , NUMBER_OF_VFOS
                                             , DISPLAY_HEADER_LINE);
   #endif
#endif   
   
   pDisplay->showVFOs(frequency_delta, currVFO);
}

/**
 * start selected vfo
 */
void initializeVFOs()   { 

   // initially all oscillators are off
   // since all the bands are driven by a single clock
   // load the starting band frequency
   si5351.output_enable(SI5351_CLK0, 0);
   vfoList[STARTING_VFO]->loadFrequency();
   delay(1); // let board settle
}

#endif // DEVICEINITIALIZATIONS_H
