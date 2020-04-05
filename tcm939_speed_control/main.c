/*****************************************************
Cassette player speed controller with CV input
*** PWM inside ***

Chip type               : ATtiny13
AVR Core Clock frequency: 9,600000 MHz
*****************************************************/

#define F_CPU 1000000L

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>

#include "lookups.h"

#define LED PB4
#define ADC_BUFFER_SIZE 8
          
ISR(TIM0_OVF_vect)
{
    static uint8_t Counter = 0;
    
    if((Counter++) > (255/(OCR0A>>4)) - 14)  //strange formula for nice LED frequency response
    {
        PORTB ^= _BV(LED);
        Counter = 0;   
    } 
}

// ADC interrupt service routine
// sampling rate: ~150 Hz, per 2 channels (~75 Hz per channel)
// averaging: 8 samples (11% of flash)
// ADMUX=0x21 - channel 1, ADMUX=0x23 - channel 3
ISR(ADC_vect)
{   
    static uint8_t ADCData1[ADC_BUFFER_SIZE]; 
    static uint8_t ADCData2[ADC_BUFFER_SIZE]; 
    static uint16_t Avg1=0;
    static uint16_t Avg2=0;  
    static uint8_t i1=0;  
    static uint8_t i2=0;
    uint16_t Mix=0; 
    uint8_t j;                      
    
    // get channel 1
    if(ADMUX==0x21)
    {
        Avg1=ADCH;
        ADCData1[i1]=ADCH;
        i1++;
        if (i1>(ADC_BUFFER_SIZE-1)) i1=0;
        for(j=0; j<=(ADC_BUFFER_SIZE-1); j++) 
            Avg1+=ADCData1[j]; 
        ADMUX|=0x02; //set to read another channel in next int 
    } 
    // get channel 3
    else
    {   
        Avg2=ADCH;
        ADCData2[i2]=ADCH;
        i2++;
        if (i2>(ADC_BUFFER_SIZE-1)) i2=0;
        for(j=0; j<=(ADC_BUFFER_SIZE-1); j++) 
            Avg2+=ADCData2[j];
        ADMUX&=0xFD; //set to read another channel in next int 
    } 
    // PW below 30% is useless, motor stops
    // ADCtoPW is a lookup table;
    // converts 0-255 value to 77-255 range in a nonlinear way...
    // ...to make control over low speeds (PW < 60%) finer    
    // add channels  
    Mix=((Avg1 + Avg2) / ADC_BUFFER_SIZE);
    if(Mix>0x00FF) Mix=0xFF;
    OCR0A = pgm_read_byte(&(ADCtoPW[Mix]));
}

//int main(void) __attribute__((noreturn));
int main(void)
{
    // Crystal Oscillator division factor: 1
    clock_prescale_set(clock_div_1);

    // Input/Output Ports initialization
    // Port B initialization
    // Func5=In Func4=Out Func3=In Func2=In Func1=In Func0=Out 
    // State5=T State4=0 State3=T State2=T State1=T State0=0 
    PORTB = 0x00;
    DDRB  = 0x11;

    // Timer/Counter 0 initialization
    // Clock source: System Clock
    // Clock value: 37,500 kHz
    // Mode: Phase correct PWM top=0xFF
    // OC0A output: Non-Inverted PWM
    // OC0B output: Disconnected
    TCCR0A = 0x81;
    TCCR0B = 0x04;
    TCNT0  = 0x00;
    OCR0A  = 0x00;
    OCR0B  = 0x00;

    // External Interrupt(s) initialization
    // INT0: Off
    // Interrupt on any change on pins PCINT0-5: Off
    GIMSK = 0x00;
    MCUCR = 0x00;

    // Timer/Counter 0 Interrupt(s) initialization
    TIMSK0 = 0x02;

    // Analog Comparator initialization
    // Analog Comparator: Off
    ACSR   = 0x80;
    ADCSRB = 0x00;
    DIDR0  = 0x00;

    // ADC initialization
    // ADC Clock frequency: 75,000 kHz
    // ADC Bandgap Voltage Reference: Off
    // ADC Auto Trigger Source: Timer0 Overflow
    // Only the 8 most significant bits of
    // the AD conversion result are used
    // Digital input buffers on ADC0: On, ADC1: On, ADC2: On, ADC3: On
    DIDR0  &= 0x03;
    DIDR0  |= 0x00;
    ADMUX   = 0x21; //Vcc reference, channel 1
    ADCSRA  = 0xAF;
    ADCSRB &= 0xF8;
    ADCSRB |= 0x04;

    // Global enable interrupts
    sei();

    for (;;) {
	// TODO does this matter?
        //sleep_mode();
    }
}
