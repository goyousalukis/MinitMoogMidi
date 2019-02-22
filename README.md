# MinitMoogMidi
A project to add MIDI in to a Moog MinitMoog

The Moog Minitmoog uses a bi-polar control voltage (CV) with a scale of 3 volts per octave(v/o). The current most commonly used standard is 1 volt per octave. I experimented with using an opamp to scale incoming 1v/o up to 3v/o with limited success.

Instead, I have decided to use an Arduino Uno to recieve MIDI in events, and output the correct voltages to the MinitMoog using a DAC chip. The Arduino uses a midi library to process incoming midi events, and then translates the note to a voltage that coresponds to 3v/o using a 12 Bit DAC.

I intend to perform a minimum of modifications to the original synthesizer, so that any future owners can easily return it to its original state.

Wish me luck...
