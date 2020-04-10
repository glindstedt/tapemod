/*****************************************************
Cassette player speed controller with CV input
*** PWM inside ***

Chip type               : ATtiny13
AVR Core Clock frequency: 9,600000 MHz
*****************************************************/

#define F_CPU 1000000L

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/sleep.h>

#include "lookups.h"

#define LED PB4
#define ADC_BUFFER_SIZE 8

ISR(TIM0_OVF_vect) {
  static uint8_t Counter = 0;
  // strange formula for nice LED frequency response
  if ((Counter++) > (255 / (OCR0A >> 4)) - 14) {
    PORTB ^= _BV(LED);
    Counter = 0;
  }
}

// ADC interrupt service routine
// sampling rate: ~150 Hz, per 2 channels (~75 Hz per channel)
// averaging: 8 samples (11% of flash)
// ADMUX=0x21 - channel 1, ADMUX=0x23 - channel 3
ISR(ADC_vect) {
  static uint8_t ADCData1[ADC_BUFFER_SIZE];
  static uint8_t ADCData2[ADC_BUFFER_SIZE];
  static uint16_t Avg1 = 0;
  static uint16_t Avg2 = 0;
  static uint8_t i1 = 0;
  static uint8_t i2 = 0;
  uint16_t Mix = 0;
  uint8_t j;

  // get channel 1
  if (ADMUX == 0x21) {
    Avg1 = ADCH;
    ADCData1[i1] = ADCH;
    i1++;
    if (i1 > (ADC_BUFFER_SIZE - 1)) {
      i1 = 0;
    }
    for (j = 0; j <= (ADC_BUFFER_SIZE - 1); j++) {
      Avg1 += ADCData1[j];
    }
    ADMUX ^= 0x02;  // flip MUX1 bit to switch to channel 3
  }
  // get channel 3
  else {
    Avg2 = ADCH;
    ADCData2[i2] = ADCH;
    i2++;
    if (i2 > (ADC_BUFFER_SIZE - 1)) {
      i2 = 0;
    }
    for (j = 0; j <= (ADC_BUFFER_SIZE - 1); j++) {
      Avg2 += ADCData2[j];
    }
    ADMUX ^= 0x02;  // flip MUX1 bit to switch to channel 1
  }
  // PW below 30% is useless, motor stops
  // ADCtoPW is a lookup table;
  // converts 0-255 value to 77-255 range in a nonlinear way...
  // ...to make control over low speeds (PW < 60%) finer
  // add channels
  Mix = ((Avg1 + Avg2) / ADC_BUFFER_SIZE);
  if (Mix > 0x00FF) {
    Mix = 0xFF;
  }
  OCR0A = pgm_read_byte(&(ADCtoPW[Mix]));
}

// int main(void) __attribute__((noreturn));
int main(void) {
  // Crystal Oscillator division factor: 1
  clock_prescale_set(clock_div_1);

  // Input/Output Ports initialization
  // Port B initialization
  // Func5=In Func4=Out Func3=In Func2=In Func1=In Func0=Out
  // State5=T State4=0 State3=T State2=T State1=T State0=0
  PORTB = 0x00;
  DDRB = 0x11;

  // Timer/Counter 0 initialization
  // Clock source: System Clock
  // Mode: Phase correct PWM top=0xFF
  // OC0A output: Non-Inverted PWM
  // OC0B output: Disconnected
  TCCR0A = 0x81;
  // Clock prescaling /1: value = 37,500 kHz
  TCCR0B = 0x01;
  // Reset registers
  TCNT0 = 0x00;
  OCR0A = 0x00;
  OCR0B = 0x00;

  // External Interrupt(s) initialization
  // INT0: Off
  // Interrupt on any change on pins PCINT0-5: Off
  GIMSK = 0x00;
  MCUCR = 0x00;

  // Timer/Counter 0 Interrupt(s) initialization
  TIMSK0 = 0x02;

  // Analog Comparator initialization
  // Analog Comparator: Off
  ACSR = 0x80;
  ADCSRB = 0x00;
  DIDR0 = 0x00;

  // ADC initialization
  // ADC Clock frequency: 75,000 kHz
  // ADC Bandgap Voltage Reference: Off
  // ADC Auto Trigger Source: Timer0 Overflow
  // Only the 8 most significant bits of
  // the AD conversion result are used
  // TODO maybe disable ADC0? does it matter?
  DIDR0 = 0x03;  // Enabled: ADC0,ADC1,ADC2,ADC3  Disabled: AIN0,AIN1

  /*
   * We set the ADLAR Left Adjust Result bit in ADMUX here:
   * > If the result is left adjusted and no more than 8-bit
   * > precision is required, it is sufficient to read ADCH
   */
  ADMUX = 0x21;  // Enable ADLAR, select ADC channel 1 (PB2)
  // Enable ADC, Auto Trigger mode, and ADC Conversion Complete Interrupt
  // ADC clock prescale division factor = 128
  ADCSRA = 0xAF;
  // Auto Trigger source selection: Timer/Counter Overflow
  //ADCSRB &= 0xF8;
  //ADCSRB |= 0x04;
  ADCSRB = 0x04;

  // Global enable interrupts
  sei();

  for (;;) {
    // TODO does this matter?
    // sleep_mode();
  }
}
