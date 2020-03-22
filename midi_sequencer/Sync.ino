void SyncIn() {
  //  cli(); //stop interrupts happening before we read pin values
  if (playStop == 1) { //send midi data if the sequence is playing
    if (firstStepDelay == 0) {
      sequenceCurrentStep++;
      if (sequenceCurrentStep > sequenceLastStep) { //wrap the sequence
        sequenceCurrentStep = sequenceFirstStep;
      }
    }
    if (noteIsOn == 1 && sequence[sequenceCurrentStep] != 129) {
      midiNoteOff(lastNote);
      noteIsOn = 0;
    }
    if (sequence[sequenceCurrentStep] < 128) {
      midiNoteOn(sequence[sequenceCurrentStep]);
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
  //  sei(); //restart interrupts
}
