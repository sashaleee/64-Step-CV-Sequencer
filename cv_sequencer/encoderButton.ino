void EncoderButton() {

  //read encoder button
  int currentEncoderButtonState = digitalRead(encoderButtonPin);
  if (currentEncoderButtonState != lastEncoderButtonState) {
    lastEncoderButtonDebounceTime = millis();
  }
  if ((millis() - lastEncoderButtonDebounceTime) > debounceDelay) {
    if (currentEncoderButtonState != encoderButtonState) {
      encoderButtonState = currentEncoderButtonState;
      if (encoderButtonState == LOW) {
        holdEncoderButtonMillis = millis();

        if (mode == 1) {
          editStep = !editStep; //togle edit step mode
          if (editStep == 1) {
            encoderPos = sequence[editStepNumber];
          } else {
            encoderPos = editStepNumber;
          }
        }

        if (mode == 4) { //select length
          sequenceFirstStep = sequenceFirstPattern * 8;
          sequenceLastStep = sequenceFirstPattern * 8 + (sequenceLengthPatterns * 8) - 1;
          sequenceCurrentStep = sequenceFirstStep;
          mode = 5;
        }

        if (mode == 3) { //select first pattern
          patternSelected = encoderPos;
          sequenceFirstPattern = patternSelected;
          sequenceLengthPatterns = 1;
          encoderPos = sequenceLengthPatterns;
          mode = 4;
        }

        if (mode == 0) {
          switch (currentMenuItem) {

            case 0:
              playStop = !playStop;
              if (playStop == 0) {
                sequenceCurrentStep = sequenceFirstStep; // go to the first step if the sequence has been stopped
                analogWrite(cvPin, 0); //output cv 0v when stopped
              }
              break;

            case 1:
              mode = 1;
              encoderPos = editStepNumber;
              break;

            case 2:
              mode = 2;
              encoderPos = BPM;
              break;

            case 3:
              saveEEPROM();
              break;

            case 4:
              mode = 3;
              encoderPos = patternSelected;
              break;
          }
        }

        if (mode == 5) { //fake mode to prevent going back to load screen right after pressing load button
          mode = 0;
        }

        updateScreen = 1;
      }
    }
  }

  //long press detection
  if (((millis() - holdEncoderButtonMillis) > holdEncoderButtonTime) && encoderButtonState == 0 && (mode == 1 || mode == 2)) {
    //exit edit mode and go to play mode
    if (playStop == 0) {
      analogWrite(cvPin, 0);
    }
    encoderPos = currentMenuItem;
    editStep = 0; //exit edit step if enabled
    mode = 0;
    updateScreen = 1;
  }
  lastEncoderButtonState = currentEncoderButtonState;
}
