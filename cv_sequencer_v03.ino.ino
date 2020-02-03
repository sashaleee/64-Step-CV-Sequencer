//*******************************************************************************************
//    hardware connections
//*******************************************************************************************
//screen SDA -> A4 ///| SCL -> A5
//encoder top left -> D3 /// top middle -> GND /// top right -> D2 /// bottom left -> D4 /// bottom right -> GND
//cv out -> D6 -> R -> (cv out & C to GND)
//play button -> D5
//sync out -> D7

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <uClock.h>
#include <EEPROM.h>

#define OLED_RESET     12 // Reset pin # (or -1 if sharing Arduino reset pin) ????
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

//*******************************************************************************************
//    pins
//*******************************************************************************************
byte pinA = 2; //first hardware interrupt pin is digital pin 2
byte pinB = 3; //second hardware interrupt pin is digital pin 3
byte buttonPin = 4;
byte playButtonPin = 5;
byte cvPin = 6;
byte syncPin = 7;

//*******************************************************************************************
//    encoder vars
//*******************************************************************************************
//encoder button
byte encoderButtonState = 0;
byte lastEncoderButtonState = HIGH;
unsigned long holdEncoderButtonMillis = 0; //how long the button is pressed
byte holdEncoderButton = 0;
int holdEncoderButtonTime = 1000;

//encoder
volatile byte aFlag = 0;
volatile byte bFlag = 0;
int encoderPos = 0;
int oldEncPos = 0;
volatile byte reading = 0;
byte editValue = 2; // 0 - change editStepNumber, 1 - change stepValue, 2 - select parameter, 3 - BPM, 4 - gate
//debounce
unsigned long lastEncoderButtonDebounceTime = 0;
unsigned long debounceDelay = 50;

//*******************************************************************************************
//    play button vars
//*******************************************************************************************
volatile byte playButtonState = 0;
volatile byte lastPlayButtonState = 0;
unsigned long lastPlayButtonDebounceTime = 0;

//*******************************************************************************************
//    clock vars
//*******************************************************************************************
byte tickCounter = 0; //counts midi pulses
byte timeDivision = 4; //go to the next step evry timeDivision pulses
int BPM = 120; //BPM rate

//*******************************************************************************************
//    screen vars
//*******************************************************************************************
byte updateScreen = 0; // redraw screen when it equals 1

//*******************************************************************************************
//    sequence vars
//*******************************************************************************************
byte sequence [64] = {
  0x01, 1, 1, 1, 1, 1, 1, 1,
  0x02, 2, 2, 2, 2, 2, 2, 2,
  0x03, 3, 3, 3, 3, 3, 3, 3,
  0x04, 4, 4, 4, 4, 4, 4, 4,
  0x05, 5, 5, 5, 5, 5, 5, 5,
  0x06, 6, 6, 6, 6, 6, 6, 6,
  0x07, 7, 7, 7, 7, 7, 7, 7,
  0x08, 8, 8, 8, 8, 8, 8, 8
}; // sequence values
byte sequenceCurrentStep = 0;
byte sequenceFirstPattern = 0;
byte sequenceLengthPatterns = 1;
byte sequenceFirstStep = sequenceFirstPattern * 8;
byte sequenceLastStep = sequenceFirstPattern * 8 + (sequenceLengthPatterns * 8) - 1;

//*******************************************************************************************
//    play/stop vars
//*******************************************************************************************
byte playStop = 0; // play mode (0 - stop, 1 - playing)
String playStopText = "PLAY";
String statusBar ="LOAD PATTERN";

//*******************************************************************************************
//    cv and gate vars
//*******************************************************************************************
float gate = timeDivision; //gate 1..12
int cv = 0;

//*******************************************************************************************
//    edit vars
//*******************************************************************************************
byte editMode = 0; // 1 - edit mode is enabled, 2 - disabled
byte editStep = 0; //edit current step of sequence, 1 -enabled, 0 - disabled
byte editStepNumber = 0;
byte saveEditStepNumber = 0; //when going to edit step value, save step number
byte stepValue = 0; //value of the current step
byte currentParameter = 0; //0..7 - active pages, 8 - BPM, 9 - gate
byte editParameter = 0; //enable editing of the current parameter
//int saveEncoderPos = 0; //keep encoder position
//int saveCurrentStep = 0; //keep current step when going to edit current step value
byte saveCurrentParameter = 0; //keep current parameter nuber when going to edit its value

//*******************************************************************************************
//    EEPROM vars
//*******************************************************************************************
byte eepromUnsaved = 0;

void setup() {

  //*******************************************************************************************
  //    pin confoguration
  //*******************************************************************************************
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(playButtonPin, INPUT_PULLUP);
  pinMode(syncPin, OUTPUT);


  //*******************************************************************************************
  //    interrupts setup
  //*******************************************************************************************
  attachInterrupt(0, PinA, RISING);
  attachInterrupt(1, PinB, RISING);

  //*******************************************************************************************
  //    serial port setup
  //*******************************************************************************************
  Serial.begin(115200); // start the serial monitor link

  //*******************************************************************************************
  //    midi clock setup
  //*******************************************************************************************
  uClock.init();
  uClock.setClock32PPQNOutput(ClockOut32PPQN);
  uClock.setTempo(BPM);
  uClock.start();

  //*******************************************************************************************
  //    load data from EEPROM
  //*******************************************************************************************
  for (int i = 0; i < 64; i++) {
    sequence[i] = EEPROM.read(i);
  }
  BPM = EEPROM.read(64);
  gate = EEPROM.read(65);
  sequenceFirstPattern = EEPROM.read(66);
  sequenceLengthPatterns = EEPROM.read(67);
  sequenceFirstStep = sequenceFirstPattern * 8;
  sequenceLastStep = sequenceFirstPattern * 8 + (sequenceLengthPatterns * 8) - 1;

  //*******************************************************************************************
  //    screen setup
  //*******************************************************************************************
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.println("V0.3");
  display.display();
  delay(1000);
  updateScreen = 1;
}

//*******************************************************************************************
//    interrupts routine
//*******************************************************************************************
void PinA() {
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos --; //decrement the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) {
    bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  }
  sei(); //restart interrupts
}

void PinB() {
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) {
    aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  }
  sei(); //restart interrupts
}

//*******************************************************************************************
//    clock
//*******************************************************************************************
void ClockOut32PPQN(uint32_t * tick) {
  tickCounter++;

  //count sequence steps and process the sequence
  if (tickCounter == timeDivision) { // go to the text step every timeDivision clock pulses and send gate and cv
    digitalWrite(syncPin, HIGH); // send sync pulse

    if (playStop == 1) {
      sequenceCurrentStep++;
      if (sequenceCurrentStep > sequenceLastStep) { //wrap the sequence
        sequenceCurrentStep = sequenceFirstStep;
      }
      //send cv
      analogWrite(cvPin, sequence[sequenceCurrentStep]);
      if (editMode == 0) {
        updateScreen = 1; //refresh screen in play mode
      }
    }

    tickCounter = 0; //wrap clock pulses counter
  } else {
    digitalWrite(syncPin, LOW);
  }

  //close gate
  if (tickCounter == gate && playStop == 1) { //close the gate according to its settings
    digitalWrite(cvPin, LOW); //send cv 0v
  }

}

void loop() {
  //*******************************************************************************************
  //    play button processing
  //*******************************************************************************************
  int currentPlayButtonState = digitalRead(playButtonPin);
  if (currentPlayButtonState != lastPlayButtonState) {
    lastPlayButtonDebounceTime = millis();
  }
  if ((millis() - lastPlayButtonDebounceTime) > debounceDelay) {
    if (currentPlayButtonState != playButtonState) {
      playButtonState = currentPlayButtonState;

      //toogle play / stop
      if (playButtonState == LOW && editMode == 0) {
        playStop = !playStop;
        //if play is dissabled close gate and turn off cv output, update label to "Play", refresh screen
        if (playStop == 0) {
          //          editStep = 0;
          playStopText = "PLAY";
          analogWrite(cvPin, 0); //output cv 0v when stopped
          updateScreen = 1;
        }

        //if play is enabled update label to "Stop"
        //refresh screen
        else {
          playStopText = "STOP";
          updateScreen = 1;
        }
      }

      //save sequence to eeprom
      if (playButtonState == LOW && editMode == 1 && eepromUnsaved == 1) {
        for (int i = 0; i < 64; i++) {
          EEPROM.write(i, sequence[i]);
        }
        EEPROM.write(64, BPM);
        EEPROM.write(65, gate);
        EEPROM.write(66, sequenceFirstPattern);
        EEPROM.write(67, sequenceLengthPatterns);
        eepromUnsaved = 0; //saved
      }
    }
  }
  lastPlayButtonState = currentPlayButtonState;

  //*******************************************************************************************
  //    encoder button processing
  //*******************************************************************************************

  //read encoder button
  int currentEncoderButtonState = digitalRead(buttonPin);
  if (currentEncoderButtonState != lastEncoderButtonState) {
    lastEncoderButtonDebounceTime = millis();
  }
  if ((millis() - lastEncoderButtonDebounceTime) > debounceDelay) {
    if (currentEncoderButtonState != encoderButtonState) {
      encoderButtonState = currentEncoderButtonState;
      if (encoderButtonState == 0) {
        holdEncoderButtonMillis = millis();

        //edit mode click------------------------------------------------------------------
        if (editMode == 1) { //encoder button has been pressed

          //togle edit step mode
          editStep = !editStep;

          //if edit step mode enabled encoder cotrols current step value
          if (editStep == 1) {
            saveEditStepNumber = editStepNumber; //save step number
            encoderPos = sequence[editStepNumber]; //load step value
            editValue = 1; //now encoder controls step value
            updateScreen = 1; //update screen
          }

          //if edit step mode has switched off
          if (editStep == 0) {
            encoderPos = saveEditStepNumber; //restore step number
            editValue = 0; //now encoder controls step number
            updateScreen = 1;
          }

        }

        //!!!
        if (editMode == 0 && currentParameter == 10) {
          editStep = 0; //exit edit step if enabled
          saveCurrentParameter = currentParameter;
          encoderPos = saveEditStepNumber;
          editValue = 0;
          editMode = 1;
          updateScreen = 1;
        }
        //!!!

        //play mode--------------------------------------------------------------------------
        if (editMode == 0 && currentParameter != 10) {
          editParameter = !editParameter;

          if (editParameter == 1) {

            //edit active patterns
            if (currentParameter >= 0 && currentParameter <= 7) {
              saveCurrentParameter = encoderPos;
              sequenceFirstPattern = saveCurrentParameter;
              sequenceLengthPatterns = 1;
              encoderPos = sequenceLengthPatterns;
              editValue = 5;
            }

            //edit BPM
            if (currentParameter == 8) {
              saveCurrentParameter = encoderPos;
              encoderPos = BPM;
              editValue = 3;
            }

            //edit gate
            if (currentParameter == 9) {
              saveCurrentParameter = encoderPos;
              encoderPos = gate;
              editValue = 4;
            }
            updateScreen = 1;
          }

          if (editParameter == 0) {
            encoderPos = saveCurrentParameter;

            if (editValue == 5) {
              sequenceFirstStep = sequenceFirstPattern * 8;
              sequenceLastStep = sequenceFirstPattern * 8 + (sequenceLengthPatterns * 8) - 1;
              sequenceCurrentStep = sequenceFirstStep;
              Serial.println(sequenceFirstStep);
              Serial.println(sequenceLastStep);
              Serial.println("---");
            }
            editValue = 2;
            updateScreen = 1;
          }
        }

      } else { //encoder button has been released
        holdEncoderButton = 0;
      }
    }
  }

  //long press detection
  if (((millis() - holdEncoderButtonMillis) > holdEncoderButtonTime) && encoderButtonState == 0 && holdEncoderButton == 0 && editMode == 1) {

    //go to play mode
    encoderPos = saveCurrentParameter;
    editStep = 0; //exit edit step if enabled
    editValue = 2;
    editMode = 0;

    holdEncoderButton = 1;
    updateScreen = 1;
  }

  lastEncoderButtonState = currentEncoderButtonState;


  //*******************************************************************************************
  //    encoder processing
  //*******************************************************************************************

  if (oldEncPos != encoderPos) {

    //step number
    if (editValue == 0) {
      if (encoderPos < 0) {
        encoderPos = 63;
      }
      if (encoderPos > 63) {
        encoderPos = 0;
      }
      editStepNumber = encoderPos;
    }

    //step value
    if (editValue == 1) {
      stepValue = encoderPos;
      sequence[editStepNumber] = stepValue;
      eepromUnsaved = 1;
    }

    //select parameter
    if (editValue == 2) {
      if (encoderPos < 0 ) {
        encoderPos = 10;
      }
      if (encoderPos > 10 ) {
        encoderPos = 0;
      }
      currentParameter = encoderPos;
    }

    //edit BPM
    if (editValue == 3) {
      if (encoderPos < 0 ) {
        encoderPos = 0;
      }
      if (encoderPos > 300 ) {
        encoderPos = 300;
      }
      BPM = encoderPos;
      uClock.setTempo(BPM);
      eepromUnsaved = 1;
    }

    //edit gate
    if (editValue == 4) {
      if (encoderPos < 1 ) {
        encoderPos = 1;
      }
      if (encoderPos > timeDivision ) {
        encoderPos = timeDivision;
      }
      gate = encoderPos;
      eepromUnsaved = 1;
    }

    //edit active steps
    if (editValue == 5) {
      if (encoderPos < 1) {
        encoderPos = 1;
      }
      if (encoderPos > 8 - sequenceFirstPattern) {
        encoderPos = 8 - sequenceFirstPattern;
      }
      sequenceLengthPatterns = encoderPos;
      eepromUnsaved = 1;
    }

    oldEncPos = encoderPos;
    updateScreen = 1;
  }

  //*******************************************************************************************
  //    output cv in edit mode when sequencer is not playing
  //*******************************************************************************************

  // !!!!!!!!!check if this working properly
  if (editMode == 1 && playStop == 0) {
    analogWrite(cvPin, sequence[editStepNumber]);
  }

  //*******************************************************************************************
  //    redraw screen
  //*******************************************************************************************

  if (updateScreen == 1) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    //edit mode
    if (editMode == 1) { //edit mode
      display.setTextSize(1);
      for (int i = 0; i < 8; i++) { //rows
        for (int n = 0; n < 8; n++) { //columns
          if (editStepNumber == ((i * 8) + n)) {
            display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
          } else {
            display.setTextColor(SSD1306_WHITE);
          }
          display.setCursor((n * 16) + 3, i * 8);
          char hexNumber[2] = {0};
          sprintf(hexNumber, "%02X", sequence[(i * 8) + n]); //convert to hex with leading zero
          display.print(hexNumber);
        }
      }
    }

    if (editMode == 0) { // play mode

      //labels
      display.setTextColor(SSD1306_WHITE);
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(statusBar);
      display.setCursor(0, 26);
      display.print("BPM");
      display.setCursor(46, 26);
      display.print("GATE");
      display.setCursor(94, 54);
      display.print(playStopText);
      display.setCursor(94, 26);
      display.print("STEP");

      //load pattern
      display.setTextSize(2);
      for (int i = 0; i < 8; i++) {
        if (i == currentParameter) {
          display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        } else {
          display.setTextColor(SSD1306_WHITE);
        }
        display.setCursor(i * 16, 6);
        if ((i >= sequenceFirstPattern) && (i < sequenceFirstPattern + sequenceLengthPatterns)) {
          display.print("x");
        } else {
          display.print("-");
        }
      }

      //BPM
      if (currentParameter == 8) {
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      display.setCursor(0, 36);
      display.print(BPM);

      //gate
      if (currentParameter == 9) {
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      display.setCursor(46, 36);
      float gateValue = (gate / timeDivision) * 100; //convert gate to percentage
      display.print(gateValue, 0);

      //edit sequence
      if (currentParameter == 10) {
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      display.setCursor(0, 54);
      display.setTextSize(1);
      display.print("EDIT SEQUENCE");

      //step
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(94, 36);
      char hexNumber[5] = {0};
      sprintf(hexNumber, "%02X", sequence[sequenceCurrentStep]); //convert to hex with leading zero
      display.setTextSize(2);
      display.print(hexNumber);
    }

    display.display();
    updateScreen = 0;
  }
}
