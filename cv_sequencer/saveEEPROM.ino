void saveEEPROM() {
  messageWindowMillis = millis();
  for (int i = 0; i < 64; i++) {
    EEPROM.write(i, sequence[i]);
  }
  EEPROM.write(64, BPM);
  EEPROM.write(66, sequenceFirstPattern);
  EEPROM.write(67, sequenceLengthPatterns);
  
  //show message window save
  messageWindowText = "SAVED";
  messageWindowState = 1;
}
