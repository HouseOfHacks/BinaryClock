# BinaryClock
Code, schematic and other documentation related to building an Arduino based BCD binary clock.

This repository will be updated as the project progresses.

V1 is a rough prototype sketch that emits BCD timer information. "Clock" starts at 0 when the Arduino is turned on and increments each second from there. Output is to the serial monitor. All that's needed is an Arduino connected to your computer.

V2 adds to V1 support for setting the time with a rotary encoder. The clock starts at 0 when the Arduino is turned on and increments each second from there. Press the rotary encoder to cycle from run mode to hour setting mode to minute setting mode to second setting mode and back to run mode. In each of the setting modes, twist the encoder to set the value. The only circuitry is a rotary encoder directly connected to the Arduino. See the sketch for which pins are used. All output is to the serial monitor.
