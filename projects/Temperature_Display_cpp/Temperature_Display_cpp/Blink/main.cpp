﻿/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>

/*End of auto generated code by Atmel studio */

//Beginning of Auto generated function prototypes by Atmel Studio
//End of Auto generated function prototypes by Atmel Studio

#include "LiquidCrystal.h"
#include "src/dht/DHT.h"

#include <avr/power.h>

// what digital pin we're connected to
// #define DHTPIN 13

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7, 10);

// Initialize DHT sensor.
DHT dht_0(2, DHT11);
DHT dht_1(13, DHT22);

int main() {
	float h;
	float t;
	
	// Initialize ArduinoUNO
	init();
	
	clock_prescale_set(clock_div_2);
	
	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);
	
	// setup serial
	Serial.begin(9600);
	
	// setup dht
	dht_0.begin();
	dht_1.begin();
	
	// print out a startup information
	lcd.clear();
	lcd.blink();
	lcd.setCursor(0, 0);
	lcd.print("Initializing...");
	Serial.print("\rInitializing...\n");
	lcd.backlight(100);
	lcd.noBlink();
	lcd.clear();

	while (true) {
		// Reading temperature or humidity takes about 250 milliseconds!
		// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
		h = dht_0.readHumidity();
		// Read temperature as Celsius (the default)
		t = dht_0.readTemperature();
		
		// Check if any reads failed and exit early (to try again).
		if (isnan(h) || isnan(t)) {
			Serial.println("Failed to read from DHT sensor!");
		} else {
			lcd.setCursor(0, 0);
			lcd.print("Hum 0: ");
			lcd.print(h);
			lcd.print(" %");
			lcd.setCursor(0, 1);
			lcd.print("Temp 0: ");
			lcd.print(t);
			lcd.print(" *C ");		
		}
		
		delay(2000);
		
		// Reading temperature or humidity takes about 250 milliseconds!
		// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
		h = dht_1.readHumidity();
		// Read temperature as Celsius (the default)
		t = dht_1.readTemperature();
		
		// Check if any reads failed and exit early (to try again).
		if (isnan(h) || isnan(t)) {
			Serial.println("Failed to read from DHT sensor!");
		} else {
			lcd.setCursor(0, 0);
			lcd.print("Hum 1: ");
			lcd.print(h);
			lcd.print(" %");
			lcd.setCursor(0, 1);
			lcd.print("Temp 1: ");
			lcd.print(t);
			lcd.print(" *C ");		
		}
		
		delay(2000);
	}
	
	return 0;
}
