# mPSD - manualProgrammableStepper-(motor)-Drive
mPSD - manualProgrammableStepper-(motor)-Driver, drive a stepper motor with a up / down simple interface

Useful for single-axis motion systems

It's based on Arduino and is designed to be simple to assembly and to put at work, without particular expertise.

There are similar project but they use a joystick or a rotative knob (aka encoder) and they cannot permit to reach an high precision and repetability during use, So I decided to reinvent the hot water :)

!!! WARNING !!! is a work in progress project !!! use at your own risk and not in production environment !!!

# Hardware

All the assembly not require soldering, but only pre-assembled modules, the hardware needed is:

- An Arduino Uno
- A LCD KeyPad Shield For Arduino
- A carrier board for A4988 / DRV8825 (called also A4988/DRV8825 Stepper Motor Control Board Expansion Board For 3D Printer)
- A 12V power supply with enough power to drive the stepper motor
- some single row straight pin header (male or female, depends to your choice and to your availability of jumper wires)
- a soldering iron for soldering the pin headers
- 6 jumper wires

and of course a stepper motor to drive

# Connections

- Place the LCD KeyPad Shield For Arduino on the Arduino Uno
- Place the A4988 or the DRV8825 board on the carrier board 

!!! PLEASE DOUBLE CHECK THE POLARITY!!! THE ORIENTATION OF THE A4988 IS DIFFERENT FROM DRV8825, THE TRIMMER IS IN THE OPPOSITE SIDE !!!
!!! WRONG POLARITY CAN DESTROY THE A4988/DRV8825 AND BOTH THE ARDUINO !!!

- Connect the stepper motor to the stepper motor connector of the carrier board
- Via jumper wires:
  - Connect the S(tep) pin of the carrier board to pin Arduino pin A1
  - Connect the D(irection) pin of the carrier board to pin Arduino pin A2
  - Connect the E(nable) pin of the carrier board to pin Arduino pin A3
  - Connect the 5V pin of the carrier board to 5V arduino pin
  - Connect the GND pin of the carrier board to GND arduino pin
  - Connect the 9V pin of the carrier board to Vin arduino pin

![alt_tag](https://github.com/alessiocavalieri/mPSD/images/mPSD_carrier_board.jpg?raw=true)

![alt_tag](https://github.com/alessiocavalieri/mPSD/images/mPSD_lcd_keypad_shield_and_arduino.jpg?raw=true)

# Implemented functions / menu items
- DRIVE (main functions, drive the motor)
- SET STEPS (set motor steps for each key press)
- SET MOTOR PWR (Set motor power behaviour, if motor is setted to be not always on, after each move the driver is disabled, motor always on require a fan on the driver)
- ABOUT

**Not yet implemented:**

- HOMING (Home up or down, needs one or two endstops connected)
- SET UNIT (Set steps per unit in order to visualize current absolute position)

