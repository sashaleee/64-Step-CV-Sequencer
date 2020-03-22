void midiNoteOn(int pitch) {
  Serial.write(144);
  Serial.write(pitch);
  Serial.write(127);
  //  Serial.print(pitch);
  //  Serial.println(" ON");
}

void midiNoteOff(int pitch) {
  Serial.write(128);
  Serial.write(pitch);
  Serial.write(0);
  //  Serial.print(pitch);
  //  Serial.println(" OFF");
}
