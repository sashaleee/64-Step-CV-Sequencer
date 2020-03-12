//==========*CV Sequencer*===================

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

#define OLED_RESET     12 // Reset pin # (or -1 if sharing Arduino reset pin) ????
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);


//midi
byte lastNote;
byte noteIsOn = 0;
byte channel;

//pins
byte pinA = 2; //first hardware interrupt pin is digital pin 2
byte pinB = 3; //second hardware interrupt pin is digital pin 3
byte encoderButtonPin = 4; //encoder button pin
byte cvPin = 6; //cv out pin
byte clockPin = 8; // clock pin
byte syncPin = 9; //sync in pin

//debounce
unsigned long lastEncoderButtonDebounceTime = 0;
unsigned long debounceDelay = 50; //buttons debounce time

//encoder
volatile byte aFlag = 0;
volatile byte bFlag = 0;
int encoderPos = 0;
int oldEncPos = 0;
volatile byte reading = 0;
byte clickCounter = 0;

byte encoderButtonState = 0; //encoder button state
byte lastEncoderButtonState = HIGH; //last encoder button state
unsigned long holdEncoderButtonMillis = 0; //how long the button is pressed
byte holdEncoderButton = 0; // 1 - long press state
int holdEncoderButtonTime = 1000; //long press time 1 second
byte mode = 0; //0 - main screen, 1 - edit sequece, 2 - edit tempo, 3 - load, 4 - sequence lrngth, 5 - fake mode
unsigned long encoderButtonFirstClickTime = 0; //first click time
int encoderButtonDoubleClickMaxTime = 500; //max time window for diuble click

//clock vars
int BPM = 100;
byte flicker = 0;
unsigned long msPerTick = 0;
int tickLength = 15;
unsigned long lastTick = 0;
unsigned long tickMillis = 0;

//sync vars
byte lastSyncReading = 0;
byte syncReading = 0;

//screen vars
byte updateScreen = 0;
unsigned long lastChangeTime = 0;
byte encoderScreenUpdateDelay = 0;
byte messageWindowState = 0;
String messageWindowText = "V0.7";
unsigned long messageWindowMillis = 0;
int messageWindowDelay = 1000;
//char noteName;

//edit mode
byte editStep = 0; //edit current step of sequence, 1 -enabled, 0 - disabled
byte editStepNumber = 0;
byte stepValue = 0; //value of the current step
byte patternSelected = 0; //0..7 - active pages, 8 - BPM, 9 - gate
byte editParameter = 0; //enable editing of the current

//menu
byte currentMenuItem = 0;

//sequence vars
byte sequence [64] = {
  60, 45, 34, 35, 45, 27, 46, 68,
  67, 20, 90, 10, 23, 0, 89, 34,
  128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128
};

//0 - 127 - notes
//128 - rest

char * noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

byte sequenceCurrentStep = 0; //curent step
byte sequenceFirstPattern = 0; //first active pattern
byte sequenceLengthPatterns = 1; //how many patterns to play after first pattern
byte sequenceFirstStep = sequenceFirstPattern * 8; //first step number 00..56
byte sequenceLastStep = sequenceFirstPattern * 8 + (sequenceLengthPatterns * 8) - 1; // last step number 7..63
byte playStop = 0; //play mode (0 - stop, 1 - playing)
byte firstStepDelay = 1; //wait for first pulse to come

void setup() {
  //==========*pins configuration*===================
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  pinMode(encoderButtonPin, INPUT_PULLUP);
  pinMode(clockPin, OUTPUT);
  pinMode(syncPin, INPUT);

  //==========*serial port*===================
  Serial.begin(31250);
  //  Serial.begin(9600);

  //==========*load data from EEPROM*===================
  for (int i = 0; i < 64; i++) {
    sequence[i] = EEPROM.read(i);
  }
  BPM = EEPROM.read(64);
  sequenceFirstPattern = EEPROM.read(66);
  sequenceLengthPatterns = EEPROM.read(67);
  sequenceFirstStep = sequenceFirstPattern * 8;
  sequenceLastStep = sequenceFirstPattern * 8 + (sequenceLengthPatterns * 8) - 1;
  sequenceCurrentStep = sequenceFirstStep;

  msPerTick = (unsigned long)1000 / (BPM * 1.0 / 60.0); //convert bpm to interval between clock ticks (milliseconds)

  //==========*screen setup*===================
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.cp437(true); //Use full 256 char 'Code Page 437' font
  updateScreen = 1;

  //==========*interrupts*===================
  attachInterrupt(0, PinA, RISING);
  attachInterrupt(1, PinB, RISING);
}

void loop() {
  Encoder();
  EncoderButton();
  BPMClock();
  SyncIn();
  UpdateScreen();
}
