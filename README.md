# Partials
* Contains two projects: `arduino-hr` in Arduino, and `fft` in Rust

## Arduino-HR
This Arduino program measures heart rate, using the KY-039 heartbeat sensor. The sensor outputs an analogue signal, which represents the light being received by a photoreceptor. This waveform is periodic, as the light received depends on whether oxygenated or deoxygenated blood is flowing. By applying a Sliding Discrete Fourier Transform on the signal, we can determine the frequency of the signal, which would be the heart rate.

## FFT
* Reference Fourier Transform implementations (including real in-place FFTs, and STFTs), based on (the publicly available):
```
@article{Sorensen1987RealvaluedFF,
  title={Real-valued fast Fourier transform algorithms},
  author={H. Sorensen and D. Jones and M. Heideman and C. Burrus},
  journal={IEEE Trans. Acoust. Speech Signal Process.},
  year={1987},
  volume={35},
  pages={849-863}
}

@INPROCEEDINGS{Ergün94testingmultivariate,
  author = {Funda Ergün},
  title = {Testing Multivariate Linear Functions: Overcoming the Generator Bottleneck},
  booktitle = {Proc. 27th STOC},
  year = {1994},
  pages = {407--416}
}
```
* <https://en.wikipedia.org/wiki/Fast_Fourier_transform#FFT_algorithms_specialized_for_real_or_symmetric_data>
* <https://ieeexplore.ieee.org/document/1165220>
* <https://www.dsprelated.com/showthread/comp.dsp/71595-1.php>

## Appendix
### Arduino-HR Schematic Diagram
![Schematic diagram of the heart rate circuit](https://raw.githubusercontent.com/patricksongzy/arduino-hr/master/images/hr_schematic.png)
