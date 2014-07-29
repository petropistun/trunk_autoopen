/*****************************************************
This program was produced by the
CodeWizardAVR V2.05.0 Professional
Automatic Program Generator
© Copyright 1998-2010 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : 
Version : 
Date    : 30.03.2014
Author  : NeVaDa
Company : 
Comments: 


Chip type               : ATmega8
Program type            : Application
AVR Core Clock frequency: 1,000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 256
*****************************************************/

#include <mega8.h>

#include <delay.h>

#include <stdio.h>

#define uint unsigned int
#define uchar unsigned char

#define ACCURACY_PER 5.0

eeprom unsigned int min_sensor_value = 0;
eeprom unsigned int max_sensor_value = 0;

/*
PINC0 - вхід від резистора
PINC4 - кнопка закриття
PINC5 - з центрального замка
PORTD4 - зумер
PORTD5 - Закриття багажника, мотор
PORTD6 - Відкриття багажника, мотор
PORTD7 - електро-муфта
PORTD0 - світодіод.
PINB.0 - кінцевик
*/
#define TIME_ZUMMER 100
#define ZUMMER_PIN  PORTD.4

#define COUNT_CHANGE_STOP 50

#define CLOSE_BUTTON PINC.4    // off=1, on=0
#define CENTRAL_BUTTON PINC.5  // off=1, on=0
#define PROG_BUTTON PINC.1     // off=1, on=0

//кінцевик
#define END_BUTTON          PINB.0     // open=0, close=1
#define END_BUTTON_CLOSE    1
#define END_BUTTON_OPEN     0

#define ELECT_COUPLING      PORTD.7

#define MOTOR_FORWARD       PORTD.5
#define MOTOR_BACKWARD      PORTD.6
#define LED                 PORTD.0

typedef enum {S_NONE, S_OPEN, S_CLOSE} Stage;

#define ACCURACY 10

char is_near(uint val1, uint val2)
{
    if (val1 <= val2 + val2*ACCURACY/100 && val1 >= val2 - val2*ACCURACY/100)
        return 1;
    else
        return 0;
}

/*void blink()
{
    LED = 1;
    delay_ms(100);
    LED = 0;
    delay_ms(50);
}    */

void StartOpen()
{
    MOTOR_FORWARD = 0;      
    MOTOR_BACKWARD = 0;
    ELECT_COUPLING = 1;
    delay_ms(100);
    MOTOR_FORWARD = 1;
}

void StartClose()
{
    MOTOR_FORWARD = 0;      
    MOTOR_BACKWARD = 0;
    ELECT_COUPLING = 1;
    delay_ms(100);
    MOTOR_BACKWARD = 1;
}

void Stop()
{
    ELECT_COUPLING = 0;
    MOTOR_FORWARD = 0;      
    MOTOR_BACKWARD = 0;
    ZUMMER_PIN = 0;
}

#define ADC_VREF_TYPE 0x40
// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input)
{
ADMUX=adc_input | (ADC_VREF_TYPE & 0xff);
// Delay needed for the stabilization of the ADC input voltage
delay_us(10);
// Start the AD conversion
ADCSRA|=0x40;
// Wait for the AD conversion to complete
while ((ADCSRA & 0x10)==0);
ADCSRA|=0x10;
return ADCW;
}

// Declare your global variables here

void main(void)
{
unsigned int prev_r_v = 0, r_v = 0, tick_count;
unsigned int time_zummer = TIME_ZUMMER;
unsigned int chs = COUNT_CHANGE_STOP; 
uchar clean = 0, i = 0;
Stage stage;
stage = S_NONE; 


// Input/Output Ports initialization
// Port B initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=P 
PORTB=0x01;
DDRB=0x00;

// Port C initialization
// Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State6=T State5=P State4=T State3=T State2=T State1=P State0=T 
PORTC=0b0101010;
DDRC=0x00;

// Port D initialization
// Func7=Out Func6=Out Func5=Out Func4=Out Func3=Out Func2=Out Func1=Out Func0=Out 
// State7=0 State6=0 State5=0 State4=0 State3=0 State2=0 State1=0 State0=0 
PORTD=0x00;
DDRD=0xFF;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: Timer 0 Stopped
TCCR0=0x00;
TCNT0=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: Timer1 Stopped
// Mode: Normal top=0xFFFF
// OC1A output: Discon.
// OC1B output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=0x00;
TCCR1B=0x00;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer2 Stopped
// Mode: Normal top=0xFF
// OC2 output: Disconnected
ASSR=0x00;
TCCR2=0x00;
TCNT2=0x00;
OCR2=0x00;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
MCUCR=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x00;

// USART initialization
// USART disabled
UCSRB=0x00;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
SFIOR=0x00;

// ADC initialization
// ADC Clock frequency: 500,000 kHz
// ADC Voltage Reference: AREF pin
ADMUX=ADC_VREF_TYPE & 0xff;
ADCSRA=0x81;

// SPI initialization
// SPI disabled
SPCR=0x00;

// TWI initialization
// TWI disabled
TWCR=0x00;
tick_count = 0; 
clean = 0;

    for( i = 0; i < 1; i++)
    {

        LED = 0; 
        delay_ms(250);
        LED = 1;
        delay_ms(150);
    }

while (1)
    {       
        tick_count++;           
        if(0 == PROG_BUTTON)
        { 
            if(1 == clean)
            {                   
                min_sensor_value = 512; //приблизно половина 
                max_sensor_value = 512; //приблизно половина
                clean = 0;
            }
                       
            if (0 == tick_count%50)
                LED = 1;
            if (0 == tick_count%100)
                LED = 0;

            //режим програмування                 
			r_v = read_adc(0);         
            
            if (0 == min_sensor_value || min_sensor_value > 1024 || r_v < min_sensor_value )
            {   
                min_sensor_value = r_v; 
            } 

            if (0 == max_sensor_value || max_sensor_value > 1024 || r_v > max_sensor_value)
            {   
                max_sensor_value = r_v; 
            }                   
        }                        
        else
        {  
            clean = 1;
             
            if (0 == min_sensor_value || min_sensor_value > 1024 || 0 == max_sensor_value || max_sensor_value > 1024)
            {
                //якщо не запрограмовано, то нічого непрацює
                LED = 1;
                delay_ms(1000);
                
                LED = 0; 
                delay_ms(500);
                
                LED = 1;
                delay_ms(250);
                
                LED = 0; 
                delay_ms(2000);

                continue;
            } 
            else
            if (is_near(min_sensor_value, max_sensor_value))
            {
                LED = 1;
                delay_ms(1000);
                
                LED = 0; 
                delay_ms(250);
                
                LED = 1;
                delay_ms(150);

                LED = 0; 
                delay_ms(250);

                LED = 1;
                delay_ms(150);
                
                LED = 0; 
                delay_ms(2000);

                continue;
            
            }   
            else
                LED = 0;
        
			if(0 == CLOSE_BUTTON)
			{  
				while(0 == CLOSE_BUTTON) delay_ms(10);
                delay_ms(10);
				while(0 == CLOSE_BUTTON) delay_ms(10);

				//Stop();
                
                delay_ms(2000);//затримка перед закриванням
                
				StartClose();   
                
				stage = S_CLOSE;
			}
			
			if(0 == CENTRAL_BUTTON && END_BUTTON_CLOSE == END_BUTTON)
			{                      
				while(0 == CENTRAL_BUTTON) delay_ms(10);
				
				Stop();
				StartOpen();
				stage = S_OPEN;
			}                 
			                 
			if (S_OPEN == stage)
			{          
				r_v = read_adc(0);
				if (r_v > max_sensor_value - max_sensor_value*ACCURACY_PER/100.0)
				{
					stage = S_NONE;   
                    //blink();
					Stop();            
				}
			}

			if (S_CLOSE == stage)
			{       
                time_zummer--;
                if(0 == time_zummer)
                {
                    ZUMMER_PIN = !ZUMMER_PIN;  
                    time_zummer = TIME_ZUMMER; 
                }                  
                
				r_v = read_adc(0);
				if (r_v < min_sensor_value)
				{       
					Stop();            
				}
                
                if (r_v == prev_r_v)
                {
                    chs --;
                    if(0 == chs)
                    {
                        Stop();           
                        chs = COUNT_CHANGE_STOP;
                    }
                }
                else
                {
                    prev_r_v = r_v;
                    chs = COUNT_CHANGE_STOP; 
                }               
                
                if(END_BUTTON_CLOSE == END_BUTTON)
                {          
                    Stop();
                }
			}
            
            if (END_BUTTON_CLOSE == END_BUTTON)
            {
                //значить закрився капот
                Stop(); // це про всяк випадок :)
                stage = S_NONE;     
            }
            
            if (S_NONE == stage && END_BUTTON_OPEN == END_BUTTON) //захист від падіння
            {
				r_v = read_adc(0);
				if (r_v < max_sensor_value - max_sensor_value*ACCURACY_PER/100.0)  
				{
    				Stop();
    				StartOpen();
    				stage = S_OPEN;
				}
            }
        }
    }
}
