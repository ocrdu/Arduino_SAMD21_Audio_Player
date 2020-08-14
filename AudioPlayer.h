const uint8_t *sampleName;
uint32_t sampleSize;

volatile uint16_t sampleNumber = 1;
volatile uint8_t sampleInterruptCounter = 0;

void playSample(const uint8_t *name, const uint32_t size) {
  sampleName = name;                              // Set global variables
  sampleSize = size;                              // for interrupt handler function
  sampleNumber = 1;                               // Start with the second sample
  sampleInterruptCounter = 0;                     // Counter for extrapolation/oversampling
  REG_TC4_CTRLA |= TC_CTRLA_ENABLE;               // Enable timer TC4
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);       // Wait for synchronization
}

void DACSetup(uint32_t sampleFreq, uint8_t bits) {
  analogWriteResolution(bits);                    // set DAC resolution 
  uint32_t top = 47972352 / (sampleFreq * 4);     // Calculate the TOP value from sample frequency
  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(1) |          // Divide the 48MHz clock source by divisor 1: 48MHz/1=48MHz
                    GCLK_GENDIV_ID(4);            // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |           // Set the duty cycle to 50/50
                     GCLK_GENCTRL_GENEN |         // Enable GCLK4
                     GCLK_GENCTRL_SRC_DFLL48M |   // Set the 48MHz clock source
                     GCLK_GENCTRL_ID(4);          // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         // Enable GCLK4 to TC4 and TC5
                     GCLK_CLKCTRL_GEN_GCLK4 |     // Select GCLK4
                     GCLK_CLKCTRL_ID_TC4_TC5;     // Feed the GCLK4 to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization
 
  REG_TC4_COUNT16_CC0 = top;                      // Set the TC4 CC0 register as the TOP value in Match Frequency Mode
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);       // Wait for synchronization

  NVIC_SetPriority(TC4_IRQn, 0);                  // Set the Nested Vector Interrupt Controller priority for TC4 to 0 (highest)
  NVIC_EnableIRQ(TC4_IRQn);                       // Connect TC4 to Nested Vector Interrupt Controller

  REG_TC4_INTFLAG |= TC_INTFLAG_OVF;              // Clear the interrupt flags
  REG_TC4_INTENSET = TC_INTENCLR_OVF;             // Enable TC4 interrupts
 
  REG_TC4_CTRLA |= TC_CTRLA_PRESCALER_DIV1 |      // Set prescaler to 1, 48MHz/1 = 48MHz
                   TC_CTRLA_WAVEGEN_MFRQ;         // Put the timer TC4 into Match Frequency Mode
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);       // Wait for synchronization
}

void TC4_Handler() {                              // Interrupt Service Routine for timer TC4
  sampleInterruptCounter++;
  if (sampleInterruptCounter == 1) {
    analogWrite(A0, (sampleName[sampleNumber] + 2 * sampleName[sampleNumber-1]) / 3);
  } else if (sampleInterruptCounter == 2) {
    analogWrite(A0, (sampleName[sampleNumber] + sampleName[sampleNumber-1]) / 2);
  } else if (sampleInterruptCounter == 3) {
    analogWrite(A0, (2 * sampleName[sampleNumber] + sampleName[sampleNumber-1]) / 3);
  } else if (sampleInterruptCounter >= 4) {
    analogWrite(A0, sampleName[sampleNumber++]);
    sampleInterruptCounter = 0;
  }
  REG_TC4_INTFLAG = TC_INTFLAG_OVF;               // Clear the OVF interrupt flag
  if (sampleNumber >= sampleSize) {               // When at the end of the sample array,
    REG_TC4_CTRLA &= ~TC_CTRLA_ENABLE;            // disable timer TC4
    while (TC4->COUNT16.STATUS.bit.SYNCBUSY);     // Wait for synchronization
  }
}