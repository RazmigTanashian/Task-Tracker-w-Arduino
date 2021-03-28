#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); //learned you can't update LCD in an ISR..
int numOfTasks;
int plusSpace = 1;
char arrow = '>';

//MUX
#define s0 4
#define s1 5
#define s2 6
#define chipEnable 8
#define muxIn 2
int muxChannel[16] = {0};

//SPI
#define serialData 11
#define latchPin 10
#define clkPin 13

/* Define our state Lookup
Input Channel ON
  E S2 S1 S0
  L L  L  L Y0 to Z
  L L  L  H Y1 to Z
  L L  H  L Y2 to Z
  L L  H  H Y3 to Z
  L H  L  L Y4 to Z
  L H  L  H Y5 to Z
  L H  H  L Y6 to Z
  L H  H  H Y7 to Z
  H X  X  X  switches off
*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  lcd.begin();
  //lcd.backlight(); not working..? just use a jumper wire to hook back light to 5V

  //=======MUX initialization
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(chipEnable, OUTPUT);
  
  pinMode(muxIn, INPUT);

  //=========SPI Initialization for shift register chip
  pinMode(latchPin, OUTPUT); //going to be opening and closing the latch pin. //SLAVE SELECT PIN
  pinMode(serialData, OUTPUT); //serial data Master will send to slave
  pinMode(clkPin, OUTPUT); // synchs data with slave
  SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, LSBFIRST, SPI_MODE0));
  digitalWrite(latchPin, LOW);
  SPI.transfer(0);
  digitalWrite(latchPin, HIGH);

  //==========UI Initialization
  initUI();

  //=========ISR enable on Port D
  // pin change interrupt control register
  //PCICR |= 0b00000100; //enable PC ISR on Port D
  //PCMSK2 |= 0b10001000; //enable PinChangeInterrupt on pin 3 & 7, Port D
}


void loop() {
  
  for(uint8_t i = 0; i < 16; i++) {
    
    digitalWrite(s0, (i & 0b001));
    digitalWrite(s1, (i & 0b010));
    digitalWrite(s2, (i & 0b100));
    digitalWrite(chipEnable, (i & 0b1000) >> 3); //when i < 8, have it write CE = 0 
    
    muxChannel[i] = digitalRead(muxIn);
      
    if(muxChannel[i] == 1) { //meaning a button was pressed along with it's corresponding Select lines
      digitalWrite(latchPin, LOW);//when we want to send data, the latchPin must be set LOW
      SPI.transfer(i);
      digitalWrite(latchPin, HIGH);//after you're done sending/receiving data, Close the latch pin, aka. bring it high

      if(muxChannel[i]) {
        lcd.setCursor(0, ((i * 2) - 1));
        lcd.print("X");
      }/*
      else if(muxChannel[i]) {
        lcd.setCursor(1, 1);
        lcd.print("X");
      }
      else {
        switch(i) {
          case 10:
            lcd.setCursor(1, 3);
            lcd.print("XX");
          case 11:
            lcd.setCursor(1, 6);
            lcd.print("XX");
          case 12:
            lcd.setCursor(1, 9);
            lcd.print("XX");
          case 13:
            lcd.setCursor(1, 12);
            lcd.print("XX");
        }
      }*/
      
    }
  }
}


void initUI() {
  lcd.print("How many tasks");
  lcd.setCursor(0, 1);
  lcd.print("do you have?");
  lcd.setCursor(13, 1);
  lcd.blink();
  numOfTasks = readNumOfTasks(); //read the function below this one!
  lcd.noBlink();
  lcd.print(numOfTasks);
  delay(900);
  lcd.clear();

  int counter = 1;

  lcd.setCursor(0, 0);
  lcd.print(arrow);
  
  for(int i = 0; i < 2; i++) {
    for(int j = 0; j < 16; j++) {
      if(i == 0 && j%2 != 0) {
        lcd.setCursor(j, i);
        lcd.print(counter);
        delay(250);
        counter++;
      }
      else if(i == 1 && j%2 != 0) {
        if(counter < 11) {
          lcd.setCursor(j, i);
          lcd.print(counter);
          delay(250);
          counter++;
        }  
        else {
          lcd.setCursor(j + plusSpace, i); plusSpace++;
          lcd.print(counter);
          delay(250);
          counter++;
        }
      }
      if(counter > numOfTasks) break; //end printing
    }
  }
  lcd.setCursor(0,0);
}

int readNumOfTasks() {
  while(1) { //keep the user here until they choose an option via button press
    
    for(uint8_t i = 0; i < 16; i++) {
      
      digitalWrite(s0, (i & 0b001));
      digitalWrite(s1, (i & 0b010));
      digitalWrite(s2, (i & 0b100));
      digitalWrite(chipEnable, (i & 0b1000) >> 3); //when i < 8, have it write CE = 0 
      
      muxChannel[i] = digitalRead(muxIn);
      
      if(muxChannel[i] == 1) { //meaning a button was pressed along with it's corresponding Select lines
        digitalWrite(latchPin, LOW);//when we want to send data, the latchPin must be set LOW
        SPI.transfer(i);
        digitalWrite(latchPin, HIGH);//after you're done sending/receiving data, Close the latch pin, aka. bring it high

        return i + 1;
      }
    }
  }
}
