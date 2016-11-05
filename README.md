# arduino-boatanchor-VFO
A variation on the original 3-band VFO sketch, designed to drive tube transmitters through an RF amp

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
 
