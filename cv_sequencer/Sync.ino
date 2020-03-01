void SyncIn() {
  syncReading = PINB & B00000010; //read pin 9
  if (syncReading != lastSyncReading) {
    if (syncReading == B00000010) {
      if (playStop == 1) { //output CV if the sequence is playing
        sequenceCurrentStep++;
        if (sequenceCurrentStep > sequenceLastStep) { //wrap the sequence
          sequenceCurrentStep = sequenceFirstStep;
        }
        analogWrite(cvPin, sequence[sequenceCurrentStep]);
        if (mode == 0) {
          updateScreen = 1; //refresh screen in play mode
        }
      }
      if (mode == 2) {
        flicker = !flicker;
        updateScreen = 1; //refresh screen in play mode
      }
    }
  }
  lastSyncReading = syncReading;
}
