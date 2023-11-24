#include <AudioPlayer.h>
#if defined (ARDUINO_SAMD_NANO_33_IOT)
#include <Arduino_LSM6DS3.h>
#endif

// Arrays holding 8-bit samples to be played at 16kHz
// and their sizes (from samples.h)
//
//  const uint8_t  HAL[] = {...};
//  const uint32_t HALSize = sizeof(HAL) / sizeof(HAL[0]);
//  const uint8_t  Eurythmics[] = {...F};
//  const uint32_t EurythmicsSize = sizeof(Eurythmics) / sizeof(Eurythmics[0]);
//  const uint8_t  Sine1kHz[] = {...};
//  const uint32_t Sine1kHzSize = sizeof(Sine1kHz) / sizeof(Sine1kHz[0]);
//
#include "samples.h"


void setup() {
  DACSetup(16000, 4);                             // Set up DAC for 16kHz playback, 4x oversampling
#if defined (ARDUINO_SAMD_NANO_33_IOT)
  IMU.begin();                                    // Start the accelerometers
#endif
}

void loop() {
#if defined (ARDUINO_SAMD_NANO_33_IOT)
  float x, y, z;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    if (x > 0.15 || x < -0.15 || y > 0.15 || y < -0.15 || z > 1.15 || z < 0.85) {
#endif
      delay(200);
      playSample(Eurythmics, EurythmicsSize);
      delay(1500);
#if defined (ARDUINO_SAMD_NANO_33_IOT)
    }
  }
#endif
}
