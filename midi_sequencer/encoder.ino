void Encoder() {
  if (oldEncPos != encoderPos) {
    lastChangeTime = millis(); //store millis of last encoder change
    encoderScreenUpdateDelay = 1; //enable 30 ms delay before calling updateScreen
    if (mode == 0) { //main screen
      if (encoderPos < 0) {
        encoderPos = 4;
      }
      if (encoderPos > 4) {
        encoderPos = 0;
      }
      currentMenuItem = encoderPos;
    }

    if (mode == 1 && editStep == 0) { //edit sequence
      if (encoderPos < 0) {
        encoderPos = 63;
      }
      if (encoderPos > 63) {
        encoderPos = 0;
      }
      editStepNumber = encoderPos;
    }

    if (mode == 1 && editStep == 1) { //edit step
      if (encoderPos < 0) {
        encoderPos = 128;
      }
      if (encoderPos == 128) {
        midiNoteOff(lastNote);
        noteIsOn = 0;
      }
      if (encoderPos > 128) {
        encoderPos = 0;
      }
      sequence[editStepNumber] = encoderPos;
    }

    if (mode == 1 && playStop == 0) {
      //      analogWrite(cvPin, sequence[editStepNumber]);
      if (noteIsOn == 1) {
        midiNoteOff(lastNote);
        noteIsOn = 0;
      }
      if (sequence[editStepNumber] != 128) {
        midiNoteOn(sequence[editStepNumber]);
        lastNote = sequence[editStepNumber];
        noteIsOn = 1;
      }
    }

    if (mode == 2) { //edit BPM
      if (encoderPos < 10) {
        encoderPos = 10;
      }
      if (encoderPos > 800) {
        encoderPos = 800;
      }
      BPM = encoderPos;
      msPerTick = (unsigned long)1000 / (BPM * 1.0 / 60.0); //convert bpm to interval between clock ticks (milliseconds)
    }

    //edit active steps
    if (mode == 3) {
      if (encoderPos < 0) {
        encoderPos = 7;
      }
      if (encoderPos > 7) {
        encoderPos = 0;
      }
      patternSelected = encoderPos;
    }

    if (mode == 4) {
      if (encoderPos < 1) {
        encoderPos = 1;
      }
      if (encoderPos > 8 - sequenceFirstPattern) {
        encoderPos = 8 - sequenceFirstPattern;
      }
      sequenceLengthPatterns = encoderPos;
    }

    oldEncPos = encoderPos;

    if (playStop == 0) { //update screen immediately if sequence is not running
      updateScreen = 1;
    }
  }

  //if sequence is running wait 30 ms after last encoder change before updating screen to reduce lag
  if ((millis() - lastChangeTime > 30) && (encoderScreenUpdateDelay == 1) && playStop == 1) {
    updateScreen = 1;
    encoderScreenUpdateDelay = 0; //disable delay
  }

}
