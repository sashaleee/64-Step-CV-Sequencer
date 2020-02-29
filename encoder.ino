void Encoder() {
  if (oldEncPos != encoderPos) {
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
      sequence[editStepNumber] = encoderPos;
    }

    if (mode == 1 && playStop == 0) {
      analogWrite(cvPin, sequence[editStepNumber]);
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
    updateScreen = 1;
  }
}
