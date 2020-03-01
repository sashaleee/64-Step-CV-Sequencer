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
  }

  if (updateScreen == 1 && messageWindowState == 0) {
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

      //play stop
      display.setTextColor(SSD1306_WHITE); // draw white text
      display.print(" ");
      int currentPattern = sequenceCurrentStep >> 3;
      display.print( 1 + currentPattern);
      display.print(":");
      display.print(1 + sequenceCurrentStep - currentPattern * 8);

      //CV
      display.fillTriangle(0, 20, 0, 64, sequence[sequenceCurrentStep] >> 1, 42, SSD1306_WHITE);
      display.setTextSize(4);
      display.setTextColor(SSD1306_INVERSE);
      display.setCursor(48, 26);
      char hexNumber[2] = {0};
      sprintf(hexNumber, "%02X", sequence[sequenceCurrentStep]); //convert to hex with leading zero
      display.print(hexNumber);

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
          char hexNumber[2] = {0};
          sprintf(hexNumber, "%02X", sequence[(n * 4 + i) + currentEditPattern * 8]); //convert to hex with leading zero
          display.print(hexNumber);
        }
      }
    }

    if (mode == 2) {//BPM screen
      display.setTextSize(3);
      display.setCursor(40, 22);
      display.print(BPM);
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
  }
}
