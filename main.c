/* 
 * File:   main.c
 * Author: Chan
 *
 * Created on November 15, 2017, 9:01 PM
 */

/*________________________________TEMP_______________________________*/

#include <p18f452.h>
#include <delays.h>
#include "xlcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "ow.h"

#pragma config OSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF

#define _XTAL_FREQ 4000000

//Global Vars
char k[1];
unsigned char TemperatureLSB;
unsigned char TemperatureMSB;
unsigned int tempint0 =0;
unsigned int tempint1 = 0;
unsigned int tempInt = 0;
float fractionFloat = 0.0000;
int fraction = 0;
unsigned char degree = 0xDF;
char tempDisplay[20];

/*---------------------------Delays---------------------------*/
void DelayFor18TCY(void)
{
    Delay1TCY();
 	Delay1TCY();
    Delay1TCY();
    Delay1TCY();
 	Delay1TCY();
    Delay1TCY();
 	Delay1TCY();
 	Delay1TCY();
 	Delay10TCYx(1);
}
 
void DelayXLCD(void)     // minimum 5ms
{
    Delay1KTCYx(5); 		// Delay of 5ms
                            // Cycles = (TimeDelay * Fosc) / 4
                            // Cycles = (5ms * 4MHz) / 4
                            // Cycles = 5,000

}

void DelayPORXLCD(void)   // minimum 15ms
{
    Delay1KTCYx(15);		// Delay of 15ms
                            // Cycles = (TimeDelay * Fosc) / 4
                            // Cycles = (15ms * 4MHz) / 4
                            // Cycles = 15,000

}

/*---------------------------LCD---------------------------*/

void LCD_setup(void){
    PORTD = 0X00;
    TRISD = 0x00;
    
    OpenXLCD(FOUR_BIT & LINES_5X7);
    while(BusyXLCD());
    SetDDRamAddr(0x00);              //Start writing at top left hand side of screen
    WriteCmdXLCD( SHIFT_DISP_LEFT );
    while(BusyXLCD());
    WriteCmdXLCD( BLINK_ON );
    while(BusyXLCD());
 }

/*------------------------------Temp------------------------------*/

void cnvtTemp(void){
    // convert Temperature
    ow_reset();                 //send reset signal to temp sensor
    ow_write_byte(0xCC);        // address all devices w/o checking ROM id.
    ow_write_byte(0x44);        //start temperature conversion 
    PORTBbits.RB3 = 1;          //power sensor from parasitic power during conversion
    Delay10KTCYx(75);           //wait the recommended 750ms for temp conversion to complete
    PORTBbits.RB3 = 0;          // turn off parasitic power

    // read Temperature
    ow_reset();                 //send reset signal to temp sensor
    ow_write_byte(0xCC);        // address all devices w/o checking ROM id.
    ow_write_byte(0xBE);        //read scratchpad 
    TemperatureLSB = ow_read_byte();   //read byte 0 which is temperature LSB
    TemperatureMSB = ow_read_byte();   //read byte 1 which is temperature MSB
    
    interpretTemp();
}

void interpretTemp(void){
    //Interpret temperature reading
    tempint0 = TemperatureLSB >> 4;          //shift bits to get the integer part of the reading
    tempint1 = TemperatureMSB << 4;
    tempInt = tempint1 |tempint0;            //combine the integer vars to get true integer reading
    
    if(TemperatureLSB & 0x01)               //mask and test bits to get fractional part of reading
        fractionFloat += 0.0625;

    if(TemperatureLSB & 0x02)
        fractionFloat += 0.125;

    if(TemperatureLSB & 0x04)
        fractionFloat += 0.25;

    if(TemperatureLSB & 0x08)
        fractionFloat += 0.5;

    fraction =fractionFloat*1000;         //convert the fraction value to an int for display
}



void disp_Temp(void){
        
    while(BusyXLCD());
    SetDDRamAddr(0x00);         //Start writing at top left hand side of screen
    while(BusyXLCD());
    Delay1KTCYx(50);            //Delay prevent screen from flickering.
    while(BusyXLCD());
    putsXLCD(tempDisplay);      // Show current temperature
    while(BusyXLCD());
    sprintf(tempDisplay,"                   ");     //clear screen
        
}


void main (void)
{ 
    // system config
    LCD_setup();
    TRISBbits.RB3=0;            //Used to trigger temp sensor for a reading
    
    
    while(1){
        cnvtTemp(void);             
        
        //string to display on screen
        sprintf(tempDisplay,"Temp: +%d.%03d%cC",tempInt,fraction,degree);
        disp_Temp();
                
        //reset vars for next reading
        tempInt = 0;
        fraction= 0;
        fractionFloat =0.0;

    }
}

