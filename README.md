# Synth utilities

These are command line programs for manipulating audio. They expect
input on stdin and produce output on stdout. They all deal with raw
32-bit float audio. Floating-point samples are nice because they can go
above 1.0 and not clip.

Eventually, the goal is/was to replace all of this with the more modular
[Truesynth project](https://github.com/graue/synth), but I continue to
use these programs, so why not release them to the world.



## Example: make something louder

    unfmt -16 <mysong.raw | amp -dB +6 | softsat | fmt -16 >louder.raw

This assumes mysong.raw is a raw waveform with 16-bit integer samples,
like a typical .wav with the header stripped off.

`unfmt -16` takes 16-bit integer samples and converts them to 32-bit
float. It's worth knowing that while 16-bit integer samples (by
definition) range from -32768 to 32767, these utilities all treat -1.0
to 1.0 as full-scale. So `unfmt -16` is not only converting, but also
dividing by 32768.0.

`amp -dB +6` increases the level by six decibels, which is about double
the amplitude (you could also use `amp -vol 2.0`, but usually dBs are
more friendly). Fun fact: While you can double an amplitude, there's not
really such a thing as "twice as loud" — the ear doesn't work that way.

`softsat` is a soft clipper. This effect lowers all samples into the
acceptable -1.0 to 1.0 range while adding less distortion than you would
get from hard clipping.

`fmt -16` does the opposite of the `unfmt` command.



## Installing

You can build each program with the provided `build` script.  Type
`./build *.c` to make them all. To put the binaries in a separate
directory, you can do something like `mkdir bin` and `cd bin` and
`../build ../*.c`.

### Endianness note

I have never tested this software on a big-endian system. `fmt` and
`unfmt` are the only programs that will care (since all the others use
floats), and you may have to change them to do what you want.



## Usage

To find out what options a program takes, run it with the `-help`
option. All programs assume 44100 KHz sample rate by default, which you
can override by setting the `RATE` or `SR` environment variable; for
example, type `export RATE=48000` in the shell before running these
programs.

### Generators [input nothing, output floats]:

* envelope: (with -generate option or if stdin is a tty)
* octagon: octagon wave (between square & tri) oscillator
* rossler: rossler oscillator
* saw: up/down saw wave oscillator
* sine: sine wave oscillator
* square: square wave oscillator
* tri: triangle wave oscillator
* white: white noise

### Time-stretching effects [input and output floats]:

* stretch: time and pitch stretcher, or resampler

I give this its own category because it is the only effect where number
of samples in and out won't be equal.

### Effects [input and output floats]:

* amp: gainer
* clip: hard clipper
* comp: peak/RMS compressor
* delay: delay line
* delay2: delay line with LFO
* deriv: differentiates input
* envelope: applies an envelope to input (can also be a generator with
  the -generate option)
* filter: IIR biquad filter (highpass, lowpass, bandpass & more)
* foldback: foldback distortion
* gate: a simple "gate"
* int: integrates input
* limit: crappy limiter, use limit2 instead
* limit2: a fairly decent lookahead limiter
* midside: left/right -> mid/side stereo converter
* pan: panner
* pink: FIR filter designed to convert white to pink noise
* power: raises all samples to a power (i.e., a distortion effect)
* reverb: a brute-force, unsophisticated but useful reverb
* sampbins: randomized sample-and-hold effect
* sfclip2: soft-clipper (avoid, softsat is better)
* shape: waveshaper distortion
* sine: with -modfreq option, input modulates sine frequency
* sineclip: weird noise effect, clips to shape of sine wave
* sinshape: strange sine-based waveshaper
* softclip: soft clipper (avoid, softsat is better)
* softsat: soft saturator / limiter
* stwidth: stereo width adjuster

### Mixers [input floats from files, output floats]:

* mix: sum waves
* mul: multiply waves

### Utilities [other kinds of input and output]:

* fix: set float samples that are NaN (not-a-number) or infinity to zero
* fmt: convert floats to integer samples
* unfmt: convert integer samples to floats



## License

This software and associated documentation files (the "Software") is
released under the CC0 Public Domain Dedication, version 1.0, as
published by Creative Commons. To the extent possible under law, the
author(s) have dedicated all copyright and related and neighboring
rights to the Software to the public domain worldwide. The Software is
distributed WITHOUT ANY WARRANTY.

If you did not receive a copy of the CC0 Public Domain Dedication along
with the Software, see
<http://creativecommons.org/publicdomain/zero/1.0/>
