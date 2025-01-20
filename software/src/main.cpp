#include <Arduino.h>

void potentiometers();
void buttons();
void controlChange(byte channel, byte control, byte value);

#define ATMEGA32U4 1 

// #define USING_BUTTONS 1

#define USING_POTENTIOMETERS 1  // comment if not using potentiometers
#ifdef ATMEGA328
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

#elif ATMEGA32U4
#include "MIDIUSB.h"

#endif

#ifdef USING_POTENTIOMETERS
#include <ResponsiveAnalogRead.h>

#endif

// BUTTONS
#ifdef USING_BUTTONS

const int N_BUTTONS = 3;                                
const int BUTTON_ARDUINO_PIN[N_BUTTONS] = { 2, 3, 4 };  

int buttonCState[N_BUTTONS] = {};
int buttonPState[N_BUTTONS] = {};

byte pin13index = 12;

// debounce
unsigned long lastDebounceTime[N_BUTTONS] = { 0 };
unsigned long debounceDelay = 50;

#endif

/////////////////////////////////////////////
// POTENTIOMETERS
#ifdef USING_POTENTIOMETERS

const int N_POTS = 4;
const int POT_ARDUINO_PIN[N_POTS] = { A0, A1, A2, A3 };

int potCState[N_POTS] = { 0 };
int potPState[N_POTS] = { 0 };
int potVar = 0;

int midiCState[N_POTS] = { 0 };
int midiPState[N_POTS] = { 0 };

const int TIMEOUT = 300;
const int varThreshold = 20;
boolean potMoving = true;
unsigned long PTime[N_POTS] = { 0 };
unsigned long timer[N_POTS] = { 0 };

int reading = 0;
float snapMultiplier = 0.01;
ResponsiveAnalogRead responsivePot[N_POTS] = {};

int potMin = 10;
int potMax = 1023;

#endif

/////////////////////////////////////////////
// MIDI
byte midiCh = 0;

/////////////////////////////////////////////
// SETUP
void setup() {

  Serial.begin(115200);

#ifdef DEBUG
  Serial.println("Debug mode");
  Serial.println();
#endif

#ifdef USING_BUTTONS
  for (int i = 0; i < N_BUTTONS; i++) {
    pinMode(BUTTON_ARDUINO_PIN[i], INPUT_PULLUP);
  }

#ifdef pin13
  pinMode(BUTTON_ARDUINO_PIN[pin13index], INPUT);
#endif

#endif

#ifdef USING_POTENTIOMETERS
  for (int i = 0; i < N_POTS; i++) {
    responsivePot[i] = ResponsiveAnalogRead(0, true, snapMultiplier);
    responsivePot[i].setAnalogResolution(1023);  // sets the resolution
  }
#endif
}

/////////////////////////////////////////////
// LOOP
void loop() {

#ifdef USING_BUTTONS
  buttons();
#endif

#ifdef USING_POTENTIOMETERS
  potentiometers();
#endif
}

/////////////////////////////////////////////
// BUTTONS
#ifdef USING_BUTTONS

//? TODO: przyciski

#endif

/////////////////////////////////////////////
// POTENTIOMETERS
#ifdef USING_POTENTIOMETERS

void potentiometers() {


  for (int i = 0; i < N_POTS; i++) {
    reading = analogRead(POT_ARDUINO_PIN[i]);
    responsivePot[i].update(reading);
    potCState[i] = responsivePot[i].getValue();

    midiCState[i] = map(potCState[i], potMin, potMax, 0, 127);

    if (midiCState[i] < 0) {
      midiCState[i] = 0;
    }
    if (midiCState[i] > 127) {
      midiCState[i] = 0;
    }

    potVar = abs(potCState[i] - potPState[i]);
    //Serial.println(potVar);

    if (potVar > varThreshold) {
      PTime[i] = millis();
    }

    timer[i] = millis() - PTime[i];

    if (timer[i] < TIMEOUT) {
      potMoving = true;
    } else {
      potMoving = false;
    }

    if (potMoving == true) {
      if (midiPState[i] != midiCState[i]) {
#ifdef ATMEGA328
        MIDI.sendControlChange(0 + i, midiCState[i], midiCh);
#elif ATMEGA32U4
        controlChange(midiCh, 0 + i, midiCState[i]);
        MidiUSB.flush();
#elif TEENSY
        usbMIDI.sendControlChange(0 + i, midiCState[i], midiCh);

#elif DEBUG
        Serial.print("Pot: ");
        Serial.print(i);
        Serial.print(" ");
        Serial.println(midiCState[i]);
#endif

        potPState[i] = potCState[i];
        midiPState[i] = midiCState[i];
      }
    }
  }
}

#endif

#ifdef ATMEGA32U4

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = { 0x0B, 0xB0 | channel, control, value };
  MidiUSB.sendMIDI(event);
}
#endif