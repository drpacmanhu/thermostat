#include <OneWire.h>
#include <DallasTemperature.h>
#include "LIFOQueue.h"
#include <Adafruit_SSD1306.h>

// DS18S20 Temperature chip i/o
const byte oneWireBus = 11;
// 3 led for the feedback
const byte powerLed = 2;
//const byte lowTemperatureLed = 4;
//const byte highTemperatureLed = 6;
// relay control pin
const byte relayControlPin = 8;
const byte decreaseButton = 9;
const byte increaseButton = 10;
// lifo queue
LiFoQueue queue(20);

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// OLED display TWI address
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(-1);

int maxTempValue = 40;
int screenSaver = 0;

void setup(void) {
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  pinMode(powerLed, OUTPUT);
  //pinMode(lowTemperatureLed, OUTPUT);
  //pinMode(highTemperatureLed, OUTPUT);
  pinMode(relayControlPin, OUTPUT);

  // Set the relays to default state
  digitalWrite(powerLed, LOW);
  digitalWrite(4, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(relayControlPin, HIGH);

  // initialize inputs/outputs
  // start serial port
  //Serial.begin(9600);
  sensors.begin();
  //set the resolution to the max
  sensors.setResolution(12);
}

void blinkPowerLed() {
  for (int i = 0; i < 6; i++) {
    digitalWrite(powerLed, HIGH);
    delay(500);
    digitalWrite(powerLed, LOW);
    delay(500);
  }
}

void changeMaxTemperature() {
  if ( (digitalRead(decreaseButton) == HIGH || digitalRead(increaseButton) == HIGH) && screenSaver >= 10 ) {
    screenSaver = 0;
  } else {
    if ( digitalRead(decreaseButton) == HIGH ) {
      display.clearDisplay();
      display.setCursor(0, 16);
      display.println("Increased");
      maxTempValue = maxTempValue + 5;
      if (maxTempValue > 80) {
        maxTempValue = 80;
      }
      display.println(maxTempValue);
      display.display();
      screenSaver = 0;
      delay(2000);
    }
    if ( digitalRead(increaseButton) == HIGH ) {
      display.clearDisplay();
      display.setCursor(0, 16);
      display.println("Decreased");
      maxTempValue = maxTempValue + -5;
      if (maxTempValue < 30) {
        maxTempValue = 30;
      }
      display.println(maxTempValue);
      display.display();
      screenSaver = 0;
      delay(2000);
    }
  }
}
void loop(void) {
  sensors.requestTemperatures(); // Send the command to get temperature readings
  double value = sensors.getTempCByIndex(0);
  if (value > -10 || value < 80) {
    queue.pushValue(value);
  }
  // we would like to have at least 15 values
  if ( queue.getSize() > 15 ) {
    //set relays
    if ( queue.getAvarage() > maxTempValue ) {
      
      digitalWrite(relayControlPin, HIGH);
    } else {
      if ( queue.getAvarage() < maxTempValue - 5 ) {
        digitalWrite(relayControlPin, LOW);
      }
    }
    display.clearDisplay();
    if (screenSaver <= 10) {
      display.setCursor(0, 16);
      display.println("Curr.Temp:");
      display.print(queue.getAvarage());
      display.println(" C");
    }
  } else {
    display.clearDisplay();
    if (screenSaver <= 10) {
      display.setCursor(0, 16);
      display.println("Queue size");
      display.println(queue.getSize());
    }
  }
  display.display();
  changeMaxTemperature();
  if (screenSaver <= 10 ) {
    screenSaver++;
  }
  //clear screen if no action was in 10 seconds
  blinkPowerLed();
}
