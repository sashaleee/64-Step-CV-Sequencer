void UpdateScreen() {
  //message window
  if (messageWindowState == 1) {
    display.clearDisplay();
    display.drawRect(2, 2, display.width() - 2, display.height() - 2, SSD1306_WHITE);
    display.setTextColor(SSD1306_WHITE); // draw white text
    display.setTextSize(2);
    display.setCursor(20, 20);
    display.print(messageWindowText);
    display.display();
    messageWindowState = 2; // message window is on
  }

  //close meassage window after 1 second
  if (millis() - messageWindowMillis > messageWindowDelay && messageWindowState == 2) {
    messageWindowState = 0;
    updateScreen = 1;
    //          Serial.println("updated");

  }

  if (updateScreen == 1 && messageWindowState == 0 && (millis() - lastUpdateTime > minUpdateDelayTime)) {
    display.clearDisplay();

    if (mode == 0) { // main screen mode
      display.setCursor(0, 0);
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);

      switch (currentMenuItem) {
        case 0:
          if (playStop == 0) {
            display.print("<PLAY>");
          } else {
            display.print("<STOP>");
          }
          break;

        case 1:
          display.print("<EDIT>");
          break;

        case 2:
          display.print("<RATE>");
          break;

        case 3:
          display.print("<SAVE>");
          break;

        case 4:
          display.print("<LOAD>");
          break;
      }

      //patternt:step
      display.setTextColor(SSD1306_WHITE); // draw white text
      display.print(" ");
      int currentPattern = sequenceCurrentStep >> 3;
      display.print( 1 + currentPattern);
      display.print(":");
      display.print(1 + sequenceCurrentStep - currentPattern * 8);

      //CV
      display.setTextSize(5);
      display.setTextColor(SSD1306_INVERSE);
      display.setCursor(32, 26);
      int noteNumber = sequence[sequenceCurrentStep];
      if (firstStepDelay == 0) {
        display.fillRect(0, 20, (1 + sequenceCurrentStep - currentPattern * 8) * 16, 44, SSD1306_WHITE);
        if (noteNumber < 128) {
          byte octave = noteNumber / 12;
          byte noteInOctave = noteNumber % 12;
          display.print(noteNames[noteInOctave]);
          display.print(octave);
        } else if (noteNumber == 128) {
          display.print("-");
        } else if (noteNumber == 129) {
          display.print("*");
        }
      } else {
        display.setCursor(0, 24);
        display.setTextSize(2);
        display.println("MIDI sequencer");
        //        display.println("------------o-------");
        //        display.println("-----o-----o-o---o--");
        //        display.println("o-o-o-o---o---o-o-o-");
        //        display.println("---o---o-o-----o----");
        //        display.print  ("--------o-----------");
      }
    }

    if (mode == 1) { //edit sequence screen mode
      display.setTextSize(2);
      byte currentEditPattern = editStepNumber >> 3; //divide current step number by 8 to get current pattern number
      display.setCursor(0, 0);
      display.setTextColor(SSD1306_WHITE);
      display.print("Pattern ");
      display.print(currentEditPattern + 1);
      if (editStep == 1) {
        display.write(7);
      }

      for (int n = 0; n < 2; n++) {
        for (int i = 0; i < 4; i++) {
          if (editStepNumber == (n * 4 + i) + currentEditPattern * 8) {
            display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
          } else {
            display.setTextColor(SSD1306_WHITE);
          }
          display.setCursor(i * 32, (n + 1) * 22);
          display.setTextSize(2);
          int noteNumber = sequence[(n * 4 + i) + currentEditPattern * 8];
          if (noteNumber < 128) {
            byte octave = noteNumber / 12;
            byte noteInOctave = noteNumber % 12;
            display.print(noteNames[noteInOctave]);
            display.setTextSize(1);
            display.print(octave);
          } else if (noteNumber == 128) {
            display.print("-");
          } else if (noteNumber == 129) {
            display.print("*");
          }
        }
      }
    }

    if (mode == 2) {//BPM screen
      //      display.setTextSize(3);
      //      display.setCursor(40, 22);
      //      display.print(BPM);
      if (flicker == 1) {
        display.fillRect(0, 0, 64, 64, SSD1306_INVERSE);

      } else {
        display.fillRect(64, 0, 128, 64, SSD1306_INVERSE);
      }
    }

    if (mode == 3 || mode == 4) {//load pattern
      display.setTextSize(2);
      for (int i = 0; i < 8; i++) {
        if (i == patternSelected) {
          display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        } else {
          display.setTextColor(SSD1306_WHITE);
        }
        display.setCursor(i * 16, 48);
        display.print(i + 1);
        if ((i >= sequenceFirstPattern) && (i < sequenceFirstPattern + sequenceLengthPatterns)) {
          //enabled
          display.fillRect(i * 16, 4, 12, 15, SSD1306_INVERSE);
        } else {
          //disabled
          display.fillRect(i * 16, 26, 12, 15, SSD1306_INVERSE);
        }
      }
    }

    display.display();
    updateScreen = 0;
    lastUpdateTime = millis();
  }
}
