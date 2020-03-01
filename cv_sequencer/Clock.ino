void BPMClock() {
  if ((millis() - lastTick) >= msPerTick) {
    tickMillis = millis(); // millis of pulse start
    lastTick += msPerTick; // next pulse
    PORTB = PORTB | B00000001; //set sync pin 8 high
  } else if (millis() - tickMillis >= tickLength) {
    PORTB = PORTB & B11111110; //set sync pin 8 low after 15 ms
  }
}
