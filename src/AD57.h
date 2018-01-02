/*
 * AD57.h
 *
 * Library for handling AD5722 / AD5732 / AD5752
 *  
 * First version created on 1-Jan 2018
 * 		Paul van Haastrecht
 *  
 * Copyright (c) 2017 Paul van Haastrecht.
 * 
 * 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 *************************************************************************************** 
 * 
 * pin usage	action
 *  1	AVss	Analog negative voltage Bipolor mode : -4 to -16V, otherwise GND
 *  2   		Not connected
 *  3	outA	Analog voltage out of DAC_A channel
 *  4			Not connected
 *  5   BIN 	Connect to VCC unless you got a clue (see remark 1)
 *  6			Not connected
 *  7	sync	connect to the slave_pin_DAC (defined in DEFINITION section below)
 *  8   clock	serial clock : connect to clk (UNO : pin 13)
 *  9   sdin	data in : connect to Mosi     (UNO : pin 11)
 * 10   LDAC	connect to ground or pin      (see remark 2)
 * 11   CLR		connect Vcc (pin 14)          (Is software controlled with OP_CLEAR)
 * 12 			Not connected
 * 
 * 13			Not Connected
 * 14	VCC		connect to 2.7V to 5.5V 
 * 15	GND		connect GND
 * 16	SDO		serial data out : connect to MISO  (UNO: pin 12)
 * 17	REFIN	connect to 2 to 3V, typically 2.5V (remark 3)
 * 18	DAC-gnd connect GND
 * 19	DAC-GND connect GND
 * 20	SIG-GND connect GND
 * 21	SIG-GND connect GND
 * 22			Not Connected
 * 23	outB	Analog voltage out of DAC_B channel
 * 24	AVdd	Analog positive voltage: +4V to +16V
 * 
 * remark 1:  BIN/2sCOMP
 * This is damn hard to understand. It defines a different output curve 
 * to follow based on the DAC values. Is it lineair or follows another 
 * curve? Honestly this setting feels like "because we can... not because 
 * you need it". As such, keep it simple and just follow lineair and tie
 * it to VCC, unless you understand (and explain me) what this is all
 * about.
 * 
 * remark 2: LDAC 
 * Tie to ground, DAC channels are individually updated when 
 * the SYNC returns from low to high. (after the write to the DAC).  
 * If you want BOTH channel to change at the same time:
 * 			1. Uncomment and set the ldac_pin_DAC (in DEFINITION section)
 * 			2. Connect the LDAC pin to this pin on the Arduino
 * 			3. BEFORE updating the DAC settings call setOutput(HOLD)
 * 		    4. update DAC_A and DAC_B
 * 			5. after updating call setOutput(RELEASE)
 * 
 * Remark 3 : REFIN
 * The simplest way to explain is that this voltage is devided by the 
 * maximum value of the ADC bits. Say REFIN = 2,5V / 10^16 = X. If a 
 * value (Y) is written in the DAC channel, the DAC-channel amplier
 * now receives a voltage value of X * Y = Z. That Z-value is the offset
 * between the lowest voltage value (AVss either negative or GND) and
 * the highest value (AVdd). It is important to have this reference 
 * voltage as stabile as possible, and you'll find that schematics 
 * use a voltage regulator. For testing however make a voltage divider. 
 * If VCC is 5V, take 2 resistors of 10K each to make it 2.5V
 * 
 *                   --------         --------
 *      +5V ---------| 10K  |-----.---| 10K   |----- GND
 *                   --------     |    --------
 *                                |
 *                               REFIN
 */

#ifndef AD57_H_
#define AD57_H_

#include <stdint.h>

/* current driver version */
#define VERSION_MAJOR 1
#define VERSION_MINOR 0

/*******************************************************************/
/** USER LEVEL AREA
 *******************************************************************
 ** DEFINITIONS
 *******************************************************************/  
/* define the type of AD57xx is connected !! select the right chip */
//#define   AD5732 1
//#define   AD5722 1
#define  AD5752	1

// MUST SET in .ino: DEFINES THE SYNC PIN CONNECTION FOR AD57XX
//#define slave_pin_DAC 9

// OPTIONAL: DEFINE FOR THE LDAC PIN ON THE AD57XX (SEE remark 2 above)
// REMOVE the comments and select the pin
//#define ldac_pin_DAC  7

/********************************************************************
 ** USER LEVEL DAC ROUTINES
 ********************************************************************/
 
/* MUST BE CALLED at begining
 * to initialize the SPI interface. For simplicity it sets range for 
 * the first DAC, which can be added / changed with setRange()*/
void dac_begin(uint8_t dac, uint8_t range);

/* get version number of driver
 * 2 nibbles
 * xxxx----   Major version number
 * ----xxxx   Minor version number
 */
uint8_t getVersion();

/* read control register value*/
uint8_t  getControl();

/* read dac channel register value*/
uint16_t getDac(uint8_t dac);

/* get channel power status 
 * return
 *  0 = DAC channel A and channel B are powered down 
 *  1 = DAC channel A powered-up
 *  2 = DAC channel B powered-up
 *  3 = DAC channel A and B are both powered-up */
uint8_t getPower();

/* read range value for dac channel */
uint8_t getRange(uint8_t dac);

/* get status information 
 *  0    			// no error / nothing set
 *  stat_err_TO  	// ERROR Therminal overheat shutdown
 *  stat_err_CA   	// ERROR Overcurrent DAC-A
 *  stat_err_CB   	// ERROR Overcurrent DAC-B
 *  stat_TS 	  	// Therminal shutdown is enabled in control
 *  stat_CLR     	// Clr is set in control register in control
 *  stat_CLAMP   	// clamp / Overcurrent protection is enabled in control
 *  stat_SDO  	 	// Serial data out is disabled  (how did you get the output ??)*/
uint16_t  getStatus();

/* setControl register
 * only when the opcode is OP_INSTR, an instruct-value is needed */
void setControl(char opcode, uint16_t instruct = 0x0);

/* set DAC channel offset value */
void setDac(uint8_t dac, uint16_t value);

/* Power-up or power-down a DAC channel*/
void setPower(uint8_t setting);

/* set DAC channel voltage range */
void setRange(uint8_t dac, uint8_t value);

/* see remark 2 above */
void setOutput(uint8_t act);

/********************************************************************
 ** USER LEVEL DAC VARIABLES
 ********************************************************************/

/* output range selections : use with setRange() */
#define p5V 		0 		// +5V
#define p10V 		1 		// +10V
#define p108V  		2 		// +10.8V
#define pn5V 		3 		// +/-5V
#define pn10V 		4		// +/-10V
#define pn108V  	5		// +/-10.8V
                                                 
/* channels use where 'dac' is requested */                                                 
#define DAC_A 		0
#define DAC_B 		2
#define DAC_AB 		4

/* control register opcode : use with setControl() */
#define OP_NOP		0		// do nothing (needed for reading)
#define OP_INSTR	1		// combine with instruction(s) (see below)
#define OP_CLEAR	4		// clear DAC values (as defined by CLR instruction)
#define OP_LOAD		5		// load new DAC values

/* control register instruction : !! Opcode MUST be set OP_INSTR !!!! */
#define	SET_TSD_ENABLE		0x8		// enable terminal shutdown command
#define SET_CLAMP_ENABLE	0x4		// enable the current-limit clamp.
#define SET_CLR_SET		    0x2		// define the DAC values after OP_CLEAR
#define SET_SDO_DISABLE		0x1		// disable the SDO output

#define	STOP_TSD_ENABLE	    0x80 	// disable terminal shutdown command
#define STOP_CLAMP_ENABLE	0x40 	// disable the current-limit clamp.
#define STOP_CLR_SET	    0x20 	// define the DAC values after OP_CLEAR
#define STOP_SDO_DISABLE	0x10 	// enable the SDO output

/* power register settings : use with setPower()*/
#define SET_PUA		0x1		// power-up DAC-A.
#define SET_PUB		0x2		// power-up DAC-B
#define STOP_PUA	0x4		// power-down DAC-A.
#define STOP_PUB	0x8		// power-down DAC-B.

/* define status feedback */
#define stat_err_TO 0x1  	// ERROR Therminal overheat shutdown
#define stat_err_CA 0x2  	// ERROR Overcurrent DAC-A
#define stat_err_CB 0x4  	// ERROR Overcurrent DAC-B
#define stat_TS 	0x8  	// Therminal shutdown is enabled in control
#define stat_CLR    0x10 	// Clr is set in control register in control
#define stat_CLAMP  0x20 	// clamp / Overcurrent protection is enabled in control
#define stat_SDO  	0x40 	// Serial data out is disabled  (how did you get the output ??)

/*******************************************/
/** INTERNAL ROUTINES AND DEFINITIONS          */
/*******************************************/

// register address
#define REG_DAC		0		// set DAC values within the range
#define REG_OUTPUT	1		// set output range
#define REG_POWER	2		// enable/disable DAC output + status bits
#define REG_CNTRL	3		// set / clear / load DAC values + control function 

/* POWER REGISTER SETTING : NOT TO BE USED BY USER PROGRAM 
 * BYTE 0 = REG selection */
#define PUA_2		0x1		// power-up DAC-A. wait > 10us with LDAC (BYTE 2)
#define PUB_2		0x4		// power-up DAC-B. wait > 10us with LDAC (BYTE 2)
#define	E_TSD 		0x20	// read only  
#define E_OCA	 	0x80	// read only  
#define E_OCB		0x200	// read only 

// internal SPI routines
void init_spi();
void write_ad(uint8_t *data);
void read_ad(uint8_t *data);

extern char slave_pin_DAC;

#endif /* AD57_H_ */
