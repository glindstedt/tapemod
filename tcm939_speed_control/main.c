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
  // TODO update formula
  // strange formula for nice LED frequency response
  if ((Counter++) > (255 / (OCR0B >> 4)) - 14) {
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

  // {{{ UPDATE CURRENT BUFFER

  // Get channel 1
  if (ADMUX == 0x21) {
    ADCData1[i1] = ADCH;
    i1++;
    if (i1 > (ADC_BUFFER_SIZE - 1)) {
      i1 = 0;
    }
  }
  // Get channel 3
  else {
    ADCData2[i2] = ADCH;
    i2++;
    if (i2 > (ADC_BUFFER_SIZE - 1)) {
      i2 = 0;
    }
  }
  ADMUX ^= 0x02;  // flip MUX1 bit to switch between channel 1 and 3

  // }}}

  // Get buffer averages
  Avg1 = 0;
  Avg2 = 0;
  for (j = 0; j <= (ADC_BUFFER_SIZE - 1); j++) {
    Avg1 += ADCData1[j];
  }
  for (j = 0; j <= (ADC_BUFFER_SIZE - 1); j++) {
    Avg2 += ADCData2[j];
  }

  // Mix
  //Avg1 = Avg1 / ADC_BUFFER_SIZE;
  //Avg2 = Avg2 / ADC_BUFFER_SIZE;
  //Mix = (Avg1 + Avg2) / 2;
  // This way of mixing saturates at 50% + 50% = 100%,
  // however 0% + 100% = 100% and 0% + 0% = 0%, which is desirable
  // and otherwise not achievable without added components
  Mix = (Avg1 + Avg2) / ADC_BUFFER_SIZE;

  // Prevent index out of bounds
  if (Mix > (ADC_TO_PW_SIZE - 1)) {
    Mix = ADC_TO_PW_SIZE - 1;
  }

  // ADCtoPW is a lookup table; convert from 0-255 index
  // Set Output Compare Register value
  OCR0B = pgm_read_byte(&(ADCtoPW[Mix]));
}

// int main(void) __attribute__((noreturn));
int main(void) {
  // Crystal Oscillator division factor: 1
  clock_prescale_set(clock_div_1);

  // Input/Output Ports initialization
  // Port B initialization
  // Func5=In Func4=Out Func3=In Func2=In Func1=In Func0=Out
  // State5=T State4=0 State3=T State2=T State1=T State0=0
  // Set internal pull-up on PB0
  PORTB = 0x01;
  // Set PB4 and PB1 as outputs
  DDRB = 0x12;

  // Timer/Counter 0 initialization
  // Clock source: System Clock
  // Mode: Phase correct PWM top=0xFF
  // OC0A output: Non-Inverted PWM
  // OC0B output: Disconnected

  // clk_IO = 9_600_000 Hz
  // For Phase Correct PWM
  // If TOP = 0xFF then a complete cycle would count up and down once
  // Given that it's an 8-bit counter this should take 2 * 2^8 = 512 cycles
  // This gives 9600000 / (2 * 2^8) = 18750 Hz
  // If we want to aim for 25 kHz for example, we would need to set TOP = OCR0A,
  // and set OCR0A to the value from this formula:
  // x = 9600000 / (2 * 25000) = 192
  // 192 in hex is 0xC0

  //TCCR0A = 0x81;  // Phase correct PWM (mode 1)
  //TCCR0A = 0x83;  // Fast PWM (mode 3)
  // Fast PWM (mode 7), TOP = OCR0A
  TCCR0A = 0x21; // Phase correct PWM (mode 5)
  // Clock prescaling /1: value = 37,500 kHz
  TCCR0B = 0x09;
  // Reset registers
  TCNT0 = 0x00;
  OCR0B = 0x00; // Used for duty cycle
  // Aim for desired frequency
  //OCR0A = 0x89;
  OCR0A = 0xA0;
  //OCR0A = 0x18;

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
  DIDR0 = 0x33;  // Enabled: ADC1,ADC3  Disabled: ADC0,ADC2,AIN0,AIN1

  /*
   * We set the ADLAR Left Adjust Result bit in ADMUX here:
   * > If the result is left adjusted and no more than 8-bit
   * > precision is required, it is sufficient to read ADCH
   */
  ADMUX = 0x21;  // Enable ADLAR, select ADC channel 1 (PB2)
  // Enable ADC, Auto Trigger mode, and ADC Conversion Complete Interrupt
  // ADC clock prescale division factor = 128
  // division factor 32 helped with glitches where pot was high but cv pulled low. maybe using a 10k pot would help if ADC conversion is slow
  ADCSRA = 0xAD;
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
