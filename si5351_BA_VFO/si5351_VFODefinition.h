#ifndef SI5351_VFODEFINITION_H
#define SI5351_VFODEFINITION_H

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
 * This file contains class definitions for si5351_VFODefinition. 
 * This class implements the methods defined in base class
 * VFODefinition, using the Etherkit SI5351 library, which may be
 * found here:
 * 
 * https://github.com/etherkit/Si5351Arduino
 */
 
#include <Arduino.h> 
#include <si5351.h>

#include "VFODefinition.h"

/**
 * This class mplements the methods defined in base class
 * VFODefinition, using the Etherkit SI5351 library.
 */
class si5351_VFODefinition : public VFODefinition {
protected:   
   
   /**
    * Stores PLL selection for this vfo
    */
   si5351_pll         si5351PLL;
   
   /**
    * Stores clock selection for this vfo
    */
   si5351_clock       si5351Clock;
   
   /**
    * Stores drive level for this vfo
    */
   si5351_drive       si5351Drive;   
   
   /**
    * reference to extenally defined SI5351 object
    * which runs this vfo
    */
   Si5351             &si5351;
   
   bool               running;
   
public:

   /**
    * Accessors for PLL
    */
   void setPLL(si5351_pll pll) {
      si5351PLL = pll;
      stop();
      si5351.set_pll(SI5351_PLL_FIXED, si5351PLL);
   }
   
   si5351_pll  getPLL() const {
      return si5351PLL;
   } 


   /**
    * Accessors for drive level
    */
   void setDrive(si5351_drive drive) {
      si5351Drive = drive;
      stop();
      si5351.drive_strength(si5351Clock, si5351Drive);
   }
   
   si5351_drive  getDrive() const {
      return si5351Drive;
   } 

   /**
    * Accessors for clock
    */
   void setClock(si5351_clock clock) {
      si5351Clock = clock;
   }
   
   si5351_clock  getClock() const {
      return si5351Clock;
   } 
   
   /**
    * Constructor 
    * 
    * @param  f       starting frequency of VFO in Hz
    * @param  minf    minimum frequency of VFO in Hz
    * @param  maxf    maximum frequency of VFO in Hz
    * @param  clock   SI5351 clock selection. See SI5351 library documentation.
    * @param  pll     SI5351 PLL selection. Defaults to PLL A.
    * @param  drive   SI5351 drive level selection. Defaults to 2 ma.
    * @param  flag    enabled/disabled state of clock. Default enabled.
    */
   si5351_VFODefinition(Si5351  &device
                      , unsigned long  f
                      , unsigned long  minf
                      , unsigned long  maxf
                      , si5351_clock   clock
                      , si5351_pll     pll   = SI5351_PLLA
                      , si5351_drive   drive = SI5351_DRIVE_2MA
                      , boolean        flag  = true)
   : si5351(device)
   , VFODefinition(f,minf,maxf,flag)
   , si5351Clock(clock)
   , si5351PLL(pll)
   , si5351Drive(drive)
   , running(false)
  {
      si5351.set_pll(SI5351_PLL_FIXED, si5351PLL);
      si5351.drive_strength(si5351Clock, si5351Drive);
      stop();
  }
   
   /**
    * Start clock running
    * Respects the vfo enabled flag, will not start clock
    * if vfo is marked disabled.
    * Only puts the request on the bus if necessary
    */
   virtual void start() {
      if (enabled && !running) {
         si5351.output_enable(si5351Clock, 1);
         running = true;      
      }  
   }
   
   /**
    * Stop clock
    * Only puts the request on the bus if necessary
    */
   virtual void stop()  {
      if (running) {
         si5351.output_enable(si5351Clock, 0);
         running = false;      
      }        
   }
   
   /**
    * loads vfo current frequency into the clock
    * takes effect immediately
    */
   virtual void loadFrequency()  {
      si5351.set_freq(
                       ((unsigned long long)(((long long) frequency) + fixedFrequencyOffset))*SI5351_FREQ_MULT
                     , SI5351_PLL_FIXED
                     , si5351Clock
      );
   }
};

#endif // SI5351_VFODEFINITION_H

