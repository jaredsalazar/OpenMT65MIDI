#include "MIDIUSB.h"                                                        //MIDI USB Library.

#define pin A3                                                              //Sustain pin

byte rows[] = {4, 6, 5, 7, 8, 9};                                           //INPUTS

byte cols[] = {2, 3, 10, A2, A1, A0, 15, 14, 16};                           //OUTPUTS

                                                                            //Midi notes codes
int notes[9][6] = {
  {24, 25, 26, 27, 28, 29,},
  {30, 31, 32, 33, 34, 35,},
  {36, 37, 38, 39, 40, 41,},
  {42, 43, 44, 45, 46, 47,},
  {48, 49, 50, 51, 52, 53,},
  {54, 55, 56, 57, 58, 59,},
  {60, 61, 62, 63, 64, 65,},
  {66, 67, 68, 69, 70, 71,},
  {72, 73, 74, 75, 76, 77,}
};


const int rowCount = sizeof(rows) / sizeof(rows[0]);                        //row size
const int colCount = sizeof(cols) / sizeof(cols[0]);                        //column size

byte keys[colCount][rowCount];                                              //actual key state
byte previousState[colCount][rowCount];                                     //previous key state
bool released[colCount][rowCount];                                          //key released state
int octive = 1;                                                             //octive value. 1(middle C). Can be + or -
int transpose = 0;                                                          //transpose value. 0(no transpose). Can be + or -


void setup() {
  Serial.begin(115200);
  delay(5000);                                                              //5 second delay for starting stability

  for (int x = 0; x < rowCount; x++) {
    pinMode(rows[x], INPUT);                                                //set row pins as inputs
  }

  for (int x = 0; x < colCount; x++) {
    pinMode(cols[x], INPUT);                                                //set column pins to INPUT to disable pins
  }

  pinMode(pin, INPUT_PULLUP);                                               //set sustain pin to INPUT PULLUP for stability
}

void loop() {
  readMatrix();
  playNote();
  delay(1);                                                                 //delay for stability
}

void readMatrix() {
  // iterate the columns
  for (int colIndex = 0; colIndex < colCount; colIndex++) {
    // col: set to output to low
    byte curCol = cols[colIndex];
    pinMode(curCol, OUTPUT);                                              //change column pin mode to output
    digitalWrite(curCol, LOW);                                            // set pin value to LOW

    // row: interate through the rows
    for (int rowIndex = 0; rowIndex < rowCount; rowIndex++) {
      byte rowCol = rows[rowIndex];
      pinMode(rowCol, INPUT_PULLUP);                                     //set row pin mode to PULLUP to keep pin values HIGH unless connected to ground
      keys[colIndex][rowIndex] = digitalRead(rowCol);
      pinMode(rowCol, INPUT);
    }
    // disable the column
    pinMode(curCol, INPUT);                                                //disable column pins
  }
}

void playNote() {
  for (int rowIndex = 0; rowIndex < rowCount; rowIndex++) {
    for (int colIndex = 0; colIndex < colCount; colIndex++) {

      if (previousState[colIndex][rowIndex] != keys[colIndex][rowIndex]) {                                //check if key pressed status has changed
        if (keys[colIndex][rowIndex] == 0) {                                                              // chack if key is pressed
          noteOn(1, notes[colIndex][rowIndex] + transpose + (octive * 12), 100, colIndex, rowIndex);      //sending NOTE ON with channel, pitch+transpose+octive, velocity, current column, and current index
          MidiUSB.flush();
        }
      }
      if (released[colIndex][rowIndex] == false && keys[colIndex][rowIndex] == 1) {
        noteOff(1, notes[colIndex][rowIndex] + transpose + (octive * 12), 100, colIndex, rowIndex);       //sending NOTE OFF withchannel, pitch+transpose+octive, velocity, current column, and current index
        MidiUSB.flush();
      }
    }
  }
  saveState();                                                                                            //saving current state
}

void saveState() {
  for (int rowIndex = 0; rowIndex < rowCount; rowIndex++) {
    for (int colIndex = 0; colIndex < colCount; colIndex++) {
      previousState[colIndex][rowIndex] = keys[colIndex][rowIndex];
    }
  }
}

void noteOn(byte channel, byte pitch, byte velocity, int colIndex, int rowIndex) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};                                     //packaging MIDI event data
  MidiUSB.sendMIDI(noteOn);                                                                               //sending MIDI data
  released[colIndex][rowIndex] = false;                                                                   //setting key pressed status
}

void noteOff(byte channel, byte pitch, byte velocity, int colIndex, int rowIndex) {
  if (analogRead(pin) > 100) {                                                                            //checking if sustain key is released. 
    midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};                                  //packaging MIDI event data
    MidiUSB.sendMIDI(noteOff);                                                                            //sending MIDI data
    released[colIndex][rowIndex] = true;                                                                  //setting key pressed status
  }
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

//void printMatrix() {
//  for (int rowIndex = 0; rowIndex < rowCount; rowIndex++) {
//    if (rowIndex < 10)
//      Serial.print(F("0"));
//    Serial.print(rowIndex); Serial.print(F(": "));
//
//    for (int colIndex = 0; colIndex < colCount; colIndex++) {
//      Serial.print(keys[colIndex][rowIndex]);
//      if (colIndex < colCount)
//        Serial.print(F(", "));
//    }
//    Serial.println("");
//  }
//  Serial.println("");
//}
