// Uses GCLK3 and TC4, affects TC5

#include "AudioPlayer.h"

const uint8_t *sampleName;
uint32_t sampleSize;
uint8_t overSampling;

void DACSetup(uint32_t sampleFreq, uint8_t overSamp) {
  if (overSamp != 1 && overSamp != 2 && overSamp != 4) {  // If oversampling is not set to 1, 2 or 4,
    overSampling = 1;                                     // default to 1 
  } else {                                                // else
    overSampling = overSamp;                              // set oversampling to 1, 2 or 4
  }
  uint32_t top = 47972352 / (sampleFreq * overSampling);  // Calculate the TOP value 
  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(1) |                  // Divide the 48MHz clock source by 1 for 48MHz
                    GCLK_GENDIV_ID(3);                    // Select GCLK3
  while (GCLK->STATUS.bit.SYNCBUSY);                      // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |                   // Set the duty cycle to 50/50
                     GCLK_GENCTRL_GENEN |                 // Enable GCLK3
                     GCLK_GENCTRL_SRC_DFLL48M |           // Set the 48MHz clock source
                     GCLK_GENCTRL_ID(3);                  // Select GCLK3
  while (GCLK->STATUS.bit.SYNCBUSY);                      // Wait for synchronization

  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |                 // Enable clock
                     GCLK_CLKCTRL_GEN_GCLK3 |             // Select GCLK3
                     GCLK_CLKCTRL_ID_TC4_TC5;             // Feed the GCLK3 to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY);                      // Wait for synchronization
 
  REG_TC4_COUNT16_CC0 = top;                              // Set the TC4 CC0 register to top
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);               // Wait for synchronization

  NVIC_SetPriority(TC4_IRQn, 0);                          // Set the interrupt priority for TC4 to 0 (highest)
  NVIC_EnableIRQ(TC4_IRQn);                               // Connect TC4 to Nested Vector Interrupt Controller

  REG_TC4_INTFLAG |= TC_INTFLAG_OVF;                      // Clear the interrupt flags
  REG_TC4_INTENSET = TC_INTENCLR_OVF;                     // Enable TC4 interrupts
 
  REG_TC4_CTRLA |= TC_CTRLA_PRESCALER_DIV1 |              // Set prescaler to 1 for 48MHz
                   TC_CTRLA_WAVEGEN_MFRQ;                 // Put the timer TC4 into Match Frequency Mode
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);               // Wait for synchronization
}

void playSample(const uint8_t *name, const uint32_t size) {
  sampleName = name;                                      // Set global variables
  sampleSize = size;                                      // for interrupt handler function
  REG_TC4_CTRLA |= TC_CTRLA_ENABLE;                       // Enable timer TC4
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);               // Wait for synchronization
}

void pauseSample() {
  REG_TC4_CTRLA &= ~TC_CTRLA_ENABLE;                      // Disable timer TC4
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);               // Wait for synchronization
}

inline void DACWrite(uint16_t sample) {
    DAC->DATA.reg = sample;                               // Shortened version
    while (ADC->STATUS.bit.SYNCBUSY);                     // of the code used
    DAC->CTRLA.bit.ENABLE = 0x01;                         // in analogWrite()
    while (ADC->STATUS.bit.SYNCBUSY);                     // in wiring_analog.c
}

void TC4_Handler() {                                      // Interrupt Service Routine for timer TC4
  static uint16_t currentSample, previousSample = 0;
  static uint8_t sampleInterruptCounter = 0;
  static uint32_t sampleNumber = 1;
  
  if (sampleInterruptCounter == 0) {                      // If this is the first pass for this sample:
    currentSample = sampleName[sampleNumber];             // Get the current sample value
    previousSample = sampleName[sampleNumber - 1];        // Get the previous sample value
    currentSample <<= 2;                                  // Go to 10 bits for calculations, 
    previousSample <<= 2;                                 // and also for sending to the DAC
  }
  
  sampleInterruptCounter++;                               // Increment the interrupt counter

  if (sampleInterruptCounter >= overSampling) {           // When interpolation has been handled:
    DACWrite(currentSample);                              // Send the current sample to the DAC
    sampleInterruptCounter = 0;                           // Reset the interrupt counter
    sampleNumber++;                                       // Go to the next sample
    if (sampleNumber >= sampleSize) {                     // At the end of the samples array:
      REG_TC4_CTRLA &= ~TC_CTRLA_ENABLE;                  // Disable timer TC4
      while (TC4->COUNT16.STATUS.bit.SYNCBUSY);           // Wait for synchronization
      sampleNumber = 1;                                   // Reset sample number to second sample
    } 
  } else
  if (sampleInterruptCounter << 1 == overSampling) {      // For 2x and 4x oversampling: middle interpolation
    DACWrite((currentSample + previousSample) >> 1);
  } else
  if (sampleInterruptCounter << 2 == overSampling) {      // For 4x oversampling: first interpolation
    DACWrite((currentSample + (3 * previousSample)) >> 2);
  } else
  if (sampleInterruptCounter == 3) {                      // For 4x oversampling: third interpolation
    DACWrite(((3 * currentSample) + previousSample) >> 2);
  }
 
  REG_TC4_INTFLAG = TC_INTFLAG_OVF;                       // Clear the OVF interrupt flag
}
