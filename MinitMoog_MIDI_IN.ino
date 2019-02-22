/////////////////////////////////////////////////////////////////////////////////
// MinitMoog Midi In MOD
// Justin Ahrens
/////////////////////////////////////////////////////////////////////////////////
//
//
// Hardware setup:
// This project uses a Visigence DAC board to provide the correct voltages to the synth
// This board has the following connections:
// V+. Gnd1, V- connect to +9v, Gnd, and - 9V ov MinitMoog via pins 1,2,3 on the Accessory Socket
// Pwr and Gnd2 - connect to ground of Arduino Uno(Nano)
// CLK (clock) connect to pin 13 (SCK)
// MOSI () connect to pin 12 (MISO)
// CS (Chip Select) connect to pin 10
//
// Connect the output of the DAC board to pin 4 of the accessory socket
// This sends the Control Voltage to the keyboard DC Summer through a 150k resistor.
//
// Connect the pin 3 of the Arduino to the OpAmp Trigger Board
// This board scales the 5V output to 8V for the trigger signal. It connects to the main board through a 1n4148 diode and a 2.2k reistor to the right side of resistor 195.
// you can solder to the leg of the resistor, or you can drill a small hole through the pcb trace, and solder a pin 
// Connect the output from the optocoupler to pin 0 (Rx)
//
// Connect the MIDI in LED to pin 3

#include <SPI.h>
#include <MIDI.h>  // Add Midi Library
#include "noteList.h" 
#include "pitches.h"

MIDI_CREATE_DEFAULT_INSTANCE();

#define SS1 (1 << 2) //Slave Select 1 PORTB
#define SS1PORT &PORTB   

#define SS2 (1 << 1) //Slave Select 2 PORTB
#define SS2PORT &PORTB

#define SS3 (1 << 0) //Slave Select 3 PORTB
#define SS3PORT &PORTB

#define SS4 (1 << 7) //Slave Select 4 PORTD
#define SS4PORT &PORTD

const int ss1 = 10;
const int ss2 = 9;
const int ss3 = 8;
const int ss4 = 7;

static const unsigned sGatePin     = 13;
//static const unsigned sAudioOutPin = 10;
static const unsigned sMaxNumNotes = 16;
MidiNoteList<sMaxNumNotes> midiNotes;

void setup() {
  // set the slaveSelectPins as an output:
  pinMode (ss1, OUTPUT);
  pinMode (ss2, OUTPUT);
  pinMode (ss3, OUTPUT);
  pinMode (ss4, OUTPUT);

  // initialize SPI:
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  writeMCP492x(2048, SS1, SS1PORT);
  
    pinMode(sGatePin,     OUTPUT);
    //pinMode(sAudioOutPin, OUTPUT);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.begin(); 
}

inline void handleGateChanged(bool inGateActive)
{
    digitalWrite(sGatePin, inGateActive ? HIGH : LOW);
}

inline void pulseGate()
{
    handleGateChanged(false);
    delay(1);
    handleGateChanged(true);
}

// -----------------------------------------------------------------------------

void handleNotesChanged(bool isFirstNote = false)
{
    if (midiNotes.empty())
    {
        handleGateChanged(false);
        //noTone(sAudioOutPin); // Remove to keep oscillator running during envelope release.
    }
    else
    {
        // Possible playing modes:
        // Mono Low:  use midiNotes.getLow
        // Mono High: use midiNotes.getHigh
        // Mono Last: use midiNotes.getLast

        byte currentNote = 0;
        if (midiNotes.getLast(currentNote))
        {
            //tone(sAudioOutPin, sNotePitches[currentNote]);
            writeMCP492x(sNotePitches[currentNote], SS1, SS1PORT);
            if (isFirstNote)
            {
                handleGateChanged(true);
            }
            else
            {
                pulseGate(); // Retrigger envelopes. Remove for legato effect.
            }
        }
    }
}

// -----------------------------------------------------------------------------

void handleNoteOn(byte inChannel, byte inNote, byte inVelocity)
{
    const bool firstNote = midiNotes.empty();
    midiNotes.add(MidiNote(inNote, inVelocity));
    handleNotesChanged(firstNote);
}

void handleNoteOff(byte inChannel, byte inNote, byte inVelocity)
{
    midiNotes.remove(inNote);
    handleNotesChanged();
}

void loop() {
  MIDI.read();
}


//Method to write to the DAC,using  digitalWrite for slave select
void writeMCP492x(uint16_t data,uint8_t slave_select) {
  // Take the top 4 bits of config and the top 4 valid bits (data is actually a 12 bit number) 
  //and OR them together
  uint8_t top_msg = (0x30 & 0xF0) | (0x0F & (data >> 8));

  // Take the bottom octet of data
  uint8_t lower_msg = (data & 0x00FF);

  // Select our DAC, Active LOW
  digitalWrite(slave_select, LOW);

  // Send first 8 bits
  SPI.transfer(top_msg);
  // Send second 8 bits
  SPI.transfer(lower_msg);

  //Deselect DAC
  digitalWrite(slave_select, HIGH);
}


//Method to write to the DAC, using direct port for slave select
void writeMCP492x(uint16_t data,uint8_t ss,volatile uint8_t* slave_port) {
  // Take the top 4 bits of config and the top 4 valid bits (data is actually a 12 bit number) 
  //and OR them together
  uint8_t top_msg = (0x30 & 0xF0) | (0x0F & (data >> 8));

  // Take the bottom octet of data
  uint8_t lower_msg = (data & 0x00FF);

  // Select our DAC, Active LOW
  *slave_port &= ~ss;

  // Send first 8 bits
  SPI.transfer(top_msg);
  // Send second 8 bits
  SPI.transfer(lower_msg);

  //Deselect DAC
  *slave_port |= ss;
}
