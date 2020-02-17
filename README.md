# Arduino-HR
This Arduino program measures heart rate, using the KY-039 hearbeat sensor. The sensor outputs an analogue signal, which represents the light being received by a photoreceptor. This waveform is periodic, as the light received depends on whether oxygenated or deoxygenated blood is flowing. By applying a Sliding Discrete Fourier Transform on the signal, we can determine the frequency of the signal, which would be the heart rate.
## Schematic Diagram
![Schematic diagram of the heart rate circuit](https://raw.githubusercontent.com/patricksongzy/arduino-hr/master/images/hr_schematic.png)
