void SyncIn() {
  syncReading = PINB & B00000010; //read pin 9
  if (syncReading != lastSyncReading) {
    if (syncReading == B00000010) {
      if (playStop == 1) { //output CV if the sequence is playing
        if (firstStepDelay == 0) {
          sequenceCurrentStep++;
          if (sequenceCurrentStep > sequenceLastStep) { //wrap the sequence
            sequenceCurrentStep = sequenceFirstStep;
          }
        }
        if (noteIsOn == 1) {
          sendNoteOff(lastNote);
          noteIsOn = 0;
        }
        if (sequence[sequenceCurrentStep] != 128) {
          sendNoteOn(sequence[sequenceCurrentStep]);
          lastNote = sequence[sequenceCurrentStep];
          noteIsOn = 1;
        }

        if (mode == 0) {
          updateScreen = 1; //refresh screen in play mode
        }

        firstStepDelay = 0;
      }
      if (mode == 2) {
        flicker = !flicker;
        updateScreen = 1; //refresh screen in play mode
      }
    }
  }
  lastSyncReading = syncReading;
}
