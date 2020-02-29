void SyncIn() {
  syncReading = PINB & B00000010; //read pin 9
  if (syncReading != lastSyncReading) {
    if (syncReading == B00000010) {

      //output CV if the sequence is playing
      if (playStop == 1) {
        sequenceCurrentStep++;
        if (sequenceCurrentStep > sequenceLastStep) { //wrap the sequence
          sequenceCurrentStep = sequenceFirstStep;
        }
        analogWrite(cvPin, sequence[sequenceCurrentStep]);
        if (mode == 0) {
          updateScreen = 1; //refresh screen in play mode
        }
      }

      PORTB = PORTB | B00000100; //set LED pin 10 high
      if (mode == 2) {
        flicker = !flicker;
        updateScreen = 1; //refresh screen in play mode
      }

    } else {
      PORTB = PORTB & B11111011; //set LED pin 10 low
    }
  }
  lastSyncReading = syncReading;
}
