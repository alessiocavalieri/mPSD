/***************************************************************************************
    mPSD - manualProgrammableStepper-(motor)-Driver, drive a stepper motor with a up / down simple interface

    Useful for single-axis motion systems

    please download from https://github.com/alessiocavalieri/mPSD
        
    Based on "LCD Button Shield Menu" - by Paul Siewert 
   
    License : This program is free software. You can redistribute it and/or modify
              it under the terms of the GNU General Public License as published by
              the Free Software Foundation, either version 3 of the License, or
              (at your option) any later version.
              This program is distributed in the hope that it will be useful,
              but WITHOUT ANY WARRANTY; without even the implied warranty of
              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
              GNU General Public License for more details.
 ***************************************************************************************/

/*
 * PINS: 
 * 
 * STEP = A1
 * DIR  = A2
 * ENA  = A3
 * 
 */


String menuItems[] = {"DRIVE", "SET STEPS", "SET MOTOR PWR", "HOMING","SET UNIT", "ABOUT"};

String version = "V.0.1 beta";
String aboutLine1 = version;
String aboutLine2 = "itink.it";

int firstStartup = 1;

int stepsPreset1;
int stepsPreset2;
int motorPowerAlwaysOn;

int stepsPreset1EEPROMAddress = 1;
int stepsPreset2EEPROMAddress = 2;
int motorPowerAlwaysOnEEPROMAddress = 3;

int pinSTEP = A1;
int pinDIR = A2;
int pinENA = A3;

int delayBetweenSteps = 10;

String LCDstring;

// Navigation button variables
int readKey;
int savedDistance = 0;

// Menu control variables
int menuPage = 0;
int maxMenuPages = round(((sizeof(menuItems) / sizeof(String)) / 2) + .5);
int cursorPosition = 0;

// Creates 3 custom characters for the menu display
byte downArrow[8] = {
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b10101, // * * *
  0b01110, //  ***
  0b00100  //   *
};

byte upArrow[8] = {
  0b00100, //   *
  0b01110, //  ***
  0b10101, // * * *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100  //   *
};

byte menuCursor[8] = {
  B01000, //  *
  B00100, //   *
  B00010, //    *
  B00001, //     *
  B00010, //    *
  B00100, //   *
  B01000, //  *
  B00000  //
};

#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal.h>

// Setting the LCD shields pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup() {

  // Initializes serial communication
  Serial.begin(9600);

  // Initializes and clears the LCD screen
  lcd.begin(16, 2);
  lcd.clear();

  // Creates the byte for the 3 custom characters
  lcd.createChar(0, menuCursor);
  lcd.createChar(1, upArrow);
  lcd.createChar(2, downArrow);

  // Read from EEPROM the saved values
  stepsPreset1 = EEPROM.read(stepsPreset1EEPROMAddress);
  stepsPreset2 = EEPROM.read(stepsPreset2EEPROMAddress);
  motorPowerAlwaysOn = EEPROM.read(motorPowerAlwaysOnEEPROMAddress);

  // Define stepper PINs as output
  pinMode(pinSTEP, OUTPUT);
  pinMode(pinDIR, OUTPUT);
  pinMode(pinENA, OUTPUT);

  // Turn all stepper driver pins OFF
  digitalWrite(pinSTEP, LOW);
  digitalWrite(pinDIR, LOW);
  digitalWrite(pinENA, HIGH); // Driver disabled at startup

}

void loop() {
  if (firstStartup){ // in first startup, enter directly in DRIVE subroutine
    menuItem1(); // DRIVE
    firstStartup = 0;
  }
  mainMenuDraw();
  drawCursor();
  operateMainMenu();
}

// This function will generate the 2 menu items that can fit on the screen. They will change as you scroll through your menu. Up and down arrows will indicate your current menu position.
void mainMenuDraw() {
  Serial.print(menuPage);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(menuItems[menuPage]);
  lcd.setCursor(1, 1);
  lcd.print(menuItems[menuPage + 1]);
  if (menuPage == 0) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  } else if (menuPage > 0 and menuPage < maxMenuPages) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  } else if (menuPage == maxMenuPages) {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
}

// When called, this function will erase the current cursor and redraw it based on the cursorPosition and menuPage variables.
void drawCursor() {
  for (int x = 0; x < 2; x++) {     // Erases current cursor
    lcd.setCursor(0, x);
    lcd.print(" ");
  }

  // The menu is set up to be progressive (menuPage 0 = Item 1 & Item 2, menuPage 1 = Item 2 & Item 3, menuPage 2 = Item 3 & Item 4), so
  // in order to determine where the cursor should be you need to see if you are at an odd or even menu page and an odd or even cursor position.
  if (menuPage % 2 == 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is even and the cursor position is even that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is even and the cursor position is odd that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
  }
  if (menuPage % 2 != 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is odd and the cursor position is even that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is odd and the cursor position is odd that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
  }
}


void operateMainMenu() {
  int activeButton = 0;
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 0: // When button returns as 0 there is no action taken
        break;
      case 1:  // This case will execute if the "forward" button is pressed
        button = 0;
        switch (cursorPosition) { // The case that is selected here is dependent on which menu page you are on and where the cursor is.
          case 0:
            menuItem1();
            break;
          case 1:
            menuItem2();
            break;
          case 2:
            menuItem3();
            break;
          case 3:
            menuItem4();
            break;
          case 4:
            menuItem5();
            break;
          case 5:
            menuItem6();
            break;
          case 6:
            menuItem7();
            break;
          case 7:
            menuItem8();
            break;
          case 8:
            menuItem9();
            break;
          case 9:
            menuItem10();
            break;
        }
        activeButton = 1;
        mainMenuDraw();
        drawCursor();
        break;
      case 2:
        button = 0;
        if (menuPage == 0) {
          cursorPosition = cursorPosition - 1;
          cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        }
        if (menuPage % 2 == 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition - 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));

        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
      case 3:
        button = 0;
        if (menuPage % 2 == 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition + 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
    }
  }
}

// This function is called whenever a button press is evaluated. The LCD shield works by observing a voltage drop across the buttons all hooked up to A0.
int evaluateButton(int x) {
  int result = 0;
  if (x < 50) {
    result = 1; // right
  } else if (x < 195) {
    result = 2; // up
  } else if (x < 380) {
    result = 3; // down
  } else if (x < 555) {
    result = 4; // left
  } else if (x < 790) {
    result = 5; // select    
  }
  return result;
}

// If there are common usage instructions on more than 1 of your menu items you can call this function from the sub
// menus to make things a little more simplified. If you don't have common instructions or verbage on multiple menus
// I would just delete this void. You must also delete the drawInstructions()function calls from your sub menu functions.
void drawInstructions() {
  lcd.setCursor(0, 1); // Set cursor to the bottom line
  lcd.print("Use ");
  lcd.write(byte(1)); // Up arrow
  lcd.print("/");
  lcd.write(byte(2)); // Down arrow
  lcd.print(" buttons");
}

void lcdPrintArrowsAndStepsPresets() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(1));
  lcd.setCursor(1, 0);
  lcd.print("-");
  lcd.setCursor(2, 0);
  lcd.write(byte(2));
  lcd.setCursor(3, 0);
  lcd.print(":" + String(stepsPreset1));
  lcd.setCursor(0, 1);
  lcd.print("<->:" + String(stepsPreset2));
  //lcd.print("<->:" + String(motorPowerAlwaysOn));
}

void lcdPrintArrowsAndMotorPowerPresets() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(1));
  lcd.setCursor(1, 0);
  lcd.print("-");
  lcd.setCursor(2, 0);
  lcd.write(byte(2));
  lcd.setCursor(3, 0);
  lcd.print(": MOTOR PWR");
  lcd.setCursor(0, 1);
  if(motorPowerAlwaysOn){
    lcd.print("ALWAYS ON");
  }
  else {
    lcd.print("OFF AFTER MOVE");
  }
}

// DRIVE SUB MENU

void menuItem1() { 
  int activeButton = 0;

  lcdPrintArrowsAndStepsPresets();

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      // RIGHT
      case 1:
        digitalWrite(pinENA, LOW); // Enable driver
        digitalWrite(pinDIR, HIGH);
        for (int step=0; step <= stepsPreset2; step++){
        digitalWrite(pinSTEP, HIGH);
        delay(delayBetweenSteps);
        digitalWrite(pinSTEP, LOW);
        }    
        if(!motorPowerAlwaysOn){ 
          digitalWrite(pinENA, HIGH); // Disable driver 
          }
        break;
      // UP  
      case 2:
        digitalWrite(pinENA, LOW); // Enable driver
        digitalWrite(pinDIR, HIGH);
        for (int step=0; step <= stepsPreset1; step++){
        digitalWrite(pinSTEP, HIGH);
        delay(delayBetweenSteps);
        digitalWrite(pinSTEP, LOW);
        }    
        if(!motorPowerAlwaysOn){ 
          digitalWrite(pinENA, HIGH); // Disable driver 
          }
        break;
      // DOWN  
      case 3:
        digitalWrite(pinENA, LOW); // Enable driver
        digitalWrite(pinDIR, LOW);
        for (int step=0; step <= stepsPreset1; step++){
        digitalWrite(pinSTEP, HIGH);
        delay(delayBetweenSteps);
        digitalWrite(pinSTEP, LOW);
        }    
        if(!motorPowerAlwaysOn){
          digitalWrite(pinENA, HIGH); // Disable driver 
          }
        break;
      // LEFT
      case 4:
        digitalWrite(pinENA, LOW); // Enable driver
        digitalWrite(pinDIR, LOW);
        for (int step=0; step <= stepsPreset2; step++){
        digitalWrite(pinSTEP, HIGH);
        delay(delayBetweenSteps);
        digitalWrite(pinSTEP, LOW);
        }    
        if(!motorPowerAlwaysOn){
          digitalWrite(pinENA, HIGH); // Disable driver 
          }
        break;    
      // SELECT - BACK BUTTON FUNCTION
      case 5:
      button = 0;
        activeButton = 1;
        break;
      default:
        Serial.print(button);
    }
  }
}

// END DRIVE SUB MENU


// SET STEPS SUB MENU

void menuItem2() { 
  int activeButton = 0;

  lcdPrintArrowsAndStepsPresets();
  
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      // RIGHT
      case 1:
        stepsPreset2=stepsPreset2+1;    
        EEPROM.write(stepsPreset2EEPROMAddress,stepsPreset2);
        lcdPrintArrowsAndStepsPresets();
        break;
      // UP          
      case 2:
        stepsPreset1=stepsPreset1+1;    
        EEPROM.write(stepsPreset1EEPROMAddress,stepsPreset1);
        lcdPrintArrowsAndStepsPresets();
        break;
      // DOWN          
      case 3:
        stepsPreset1=stepsPreset1-1;
        EEPROM.write(stepsPreset1EEPROMAddress,stepsPreset1);        
        lcdPrintArrowsAndStepsPresets();
        break;
      // LEFT        
      case 4:
        stepsPreset2=stepsPreset2-1;    
        EEPROM.write(stepsPreset2EEPROMAddress,stepsPreset2);
        lcdPrintArrowsAndStepsPresets();
        break;    
      // SELECT - BACK BUTTON FUNCTION        
      case 5:
        button = 0;
        activeButton = 1;
        break;
      default:
        Serial.print(button);
    }
  }
}

// END SET STEPS SUB MENU

// SET MOTOR PWR SUB MENU

void menuItem3() {
  int activeButton = 0;

  lcdPrintArrowsAndMotorPowerPresets();

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      // RIGHT
      case 1:
        break;
      // UP          
      case 2:
        motorPowerAlwaysOn=1;    
        EEPROM.write(motorPowerAlwaysOnEEPROMAddress,motorPowerAlwaysOn);
        lcdPrintArrowsAndMotorPowerPresets();
        digitalWrite(pinENA, LOW); // Enable driver 
        break;
      // DOWN          
      case 3:
        motorPowerAlwaysOn=0;    
        EEPROM.write(motorPowerAlwaysOnEEPROMAddress,motorPowerAlwaysOn);
        lcdPrintArrowsAndMotorPowerPresets();
        digitalWrite(pinENA, HIGH); // Disable driver 
        break;
      // LEFT        
      case 4:
        break;    
      // SELECT - BACK BUTTON FUNCTION        
      case 5:
        button = 0;
        activeButton = 1;
        break;
      default:
        Serial.print(button);
    }
  }
}

// END SET MOTOR PWR SUB MENU

void menuItem4() { // Function executes when you select the 4th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 4");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem5() { // Function executes when you select the 5th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 5");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem6() { // Function executes when you select the 6th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.setCursor(1, 0);
  lcd.print(aboutLine1);
  lcd.setCursor(1, 1);
  lcd.print(aboutLine2);

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 5:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem7() { // Function executes when you select the 7th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 7");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem8() { // Function executes when you select the 8th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 8");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem9() { // Function executes when you select the 9th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 9");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem10() { // Function executes when you select the 10th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 10");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

