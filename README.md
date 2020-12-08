## A simple (but oversampling) audio player for SAMD21G-based Arduinos

Basically, this library lets you send an array of 8-bit samples to the DAC on pin A0, with optional extrapolation aka oversampling.

I have connected the output pin through a 2.2μF capacitor and a 1kΩ resistor in series to a small and cheap active speaker set like you would use for a computer. It also seems to work fine when I connect it to the AUX input of the sound thingy on my computer.

### Notes:

* Plays 8-bit samples in mono only. Extrapolation aka oversampling can be used to improve the output.

* Uses GCLK3 and TC4, affects TC5.

* The values of the capacitor and resistor I use have not been calculated; they just happened to be the ones lying around within reach and their values seemed close enough for this.

* Sound quality may improve a bit if you add a low-pass filter to get rid of the "steps" in the output. Extrapolation does make them smaller, but they are still there.

* If you have an Arduino 33 IoT, playing the sound with the example will start when you tap your breadboard, or hit your table (Tilt!). Other Arduinos will just loop the sample with a short pause inbetween (I hope).

* The example will load very slowly in the Arduino IDE, because there is a large sample array in it, and the Arduino IDE isn't very good at loading large programs quickly.

* Only tested on the Arduino Nano 33 IoT, and this is an early version; YMMV.

### Thanks:

Thanks go to Martin L. for explaining timer-related esoteric and dark stuff on the forums.

### Usage:

Include: `#include <AudioPlayer.h>`

In setup: `DACSetup(f, n);`  for f Hz playback with n times oversampling (n = 1, 2 or 4).

To play audio: `playSample(arrayname, arraysize);`

Also see the code.

If you feel like donating for this, you can do so here: http://ocrdu.nl/donations .