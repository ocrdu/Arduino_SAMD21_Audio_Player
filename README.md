## A simple (but oversampling) audio player for SAMD21-based Arduinos

Basically, this program sends an array of 8-bit samples to the DAC on pin A0. I have connected this pin through a 2.2μF capacitor and a 1kΩ resistor in series to a small and cheap active speaker set like you would use for a computer. It also seems to work fine when I connect it to the AUX input of the sound thingy on my computer.

### Notes:

* Plays 8-bit samples only. Extrapolation aka oversampling is used to improve the output.

* Uses GCLK4 and TC4, affects TC5.

* The values of the capacitor and resistor I use have not been calculated; they just happened to be the ones lying around within reach and their values seemed close enough for this.

* Sound quality may improve a bit if you add a low-pass filter to get rid of the "steps" in the output. Oversampling does make them smaller, but they are still there.

* If you have an Arduino 33 IoT, playing the sound will start when you tap your breadboard, or hit your table (Tilt!). Other Arduinos will just loop the sample with a short pause inbetween (I hope).

* This is an early version; I may turn it into a proper library someday, who knows.

### Thanks:

Thanks go to Martin L. for explaining timer-related esoteric and dark stuff on the forums.

### Usage:

Include: `#include "AudioPlayer.h"`

In setup: `DACSetup(f, n);`  for f Hz playback with n times oversampling (n = 1, 2 or 4).

To play audio: `playSample(arrayname, arraysize);`

Also see the code.

If you feel like donating for this, you can do so here: http://ocrdu.nl/donations .