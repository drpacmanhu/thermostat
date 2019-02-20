#include <OneWire.h>
#include <DallasTemperature.h>
#include "LIFOQueue.h"

// DS18S20 Temperature chip i/o
const byte oneWireBus = 10;
// 3 led for the feedback
const byte powerLed = 2;
const byte lowTemperatureLed = 4;
const byte highTemperatureLed = 6;
// relay control pin
const byte relayControlPin = 8;
// lifo queue
LiFoQueue queue(20);

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void setup(void) {
  pinMode(powerLed, OUTPUT);
  pinMode(lowTemperatureLed, OUTPUT);
  pinMode(highTemperatureLed, OUTPUT);
  pinMode(relayControlPin, OUTPUT);

  // Set the relays to default state
  digitalWrite(powerLed, LOW);
  digitalWrite(lowTemperatureLed, HIGH);
  digitalWrite(highTemperatureLed, HIGH);
  digitalWrite(relayControlPin, HIGH);

  // initialize inputs/outputs
  // start serial port
  Serial.begin(9600);
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
void loop(void) {
  for (int i = 0; i < 20; i++) {
    sensors.requestTemperatures(); // Send the command to get temperature readings
    double value = sensors.getTempCByIndex(0);
    Serial.print("sensor reading: ");
    Serial.println(value);

    if (value < -10 || value > 80) {
      Serial.println("Rejecting data due to its possible invalidity!");
    } else {
      queue.pushValue(value);
    }
    // we would like to have at least 15 values
    if ( queue.getSize() > 15 ) {
      //set the leds and relays
      if ( queue.getAvarage() > 40 ) {
        Serial.println("We are at the required temperature switching relay OFF...");
        digitalWrite(lowTemperatureLed, LOW);
        digitalWrite(highTemperatureLed, LOW);
        digitalWrite(relayControlPin, HIGH);
      } else {
        if ( queue.getAvarage() > 35 ) {
          Serial.println("We are IN the required temperature range switching relay OFF...");
          digitalWrite(lowTemperatureLed, HIGH);
          digitalWrite(highTemperatureLed, LOW);
        } else {
          Serial.println("We are UNDER the required temperature switching relay ON...");
          digitalWrite(lowTemperatureLed, LOW);
          digitalWrite(highTemperatureLed, HIGH);
          digitalWrite(relayControlPin, LOW);
        }
      }
    }
    //delay(5000);
    blinkPowerLed();
  }
}
