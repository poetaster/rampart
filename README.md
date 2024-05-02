# rampart
An arduino semi-modular synth companion to keep, moat and bastl's kastle.

The prototype uses an Arduino Nano. The Mozzi library is used for synthesis.

![V3 top pcb](rampartv3_pcb.jpg)

## Hardware

* 3 pots
* 1 encoder w/button
* 2 buttons
* tactile switch w/LED
* CV inputs (analog)
* header for a SSD1306 display 
* analog filter/volume control
* audio/clock in
* audio wired stereo out
* headers for most digital and analog pins

Analog Pins 0-2 are potentionmeters. Analog pins 3-7 (4 avaible as an input) are used for modulation signal inputs. 

Pins 4&5, also SCL/SDA are layed out such that a SSD1306 could be used for a display.

Digital Pin 9 is audio (PWM) out, the Mozzi standard. 

Digital 2 - 6 are used for two buttons, an encoder and encoder button.

The layout includes an analog lowpass/volume knob and analog clock/audio in (pocket operator compatible).

The two buttons are located on the top pcb as is a 3pin header which is layed out GND/IO/VCC for connecting things like touch panels.

## Fritzing 

There are two Fritzing files used, one for the main pcb and one for the top (pictured above).

## Arduino Sketches

The currently most complete sketch is the rampart3-bytebeat sketch. 

A number of Arduino sketches being updated include 2 synths (switch with button 3 on digital pin 2) which have 3 to 4 variable parts of the signal. Both sketches share the wavepacket object as 1st synth. One has PDResonant as second, one an FM synth as second synth.

So, Patch 1 is always a wavelet (based on the wavelet example with mozzi) synth and Patch 2 is an FM synth or PDResonant. Also still rough ideas.

This synth works well with Keep (moat), my analog synth and also with Bastl's Kastle synth. I've also played it with Korg's micro modular which works quiet well (in both directions).

I've included a granular synth without mozzi as an example and two versions of a 'glitch synth' from other sources modified to work with rampart.

## Physical inputs

A note on -/+ located to the left below the volume knob ... these can also be used as input ;) If you take an internal signal (say my moat or a korg micromodular) you can use the filter to simply 'mix' the signals.

# Historical versions

![laser cut and etched prototype, v2](rampart_v2.jpg)

![BreadBoard view](https://github.com/poetaster/rampart/raw/master/rampart_bb.jpg)


![PCB view](https://github.com/poetaster/rampart/raw/master/rampart_pcb.jpg)
