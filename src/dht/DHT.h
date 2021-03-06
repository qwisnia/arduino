/* DHT library

MIT license
written by Adafruit Industries
modified by qwisnia
*/
#ifndef DHT_H
#define DHT_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


// Uncomment to enable printing out nice debug messages.
//#define DHT_DEBUG

// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

#define MIN_INTERVAL 2000

// Setup debug printing macros.
#ifdef DHT_DEBUG
#define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
#define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
#define DEBUG_PRINT(...) {}
#define DEBUG_PRINTLN(...) {}
#endif

// macro which tells if a timeout happened.
// Takes three parameters:
// start_time: time when user started measuring time, it is time from the past.
//             Usually measured using micros() or mills().
// current_time: time to verify timeout against.
//               Usually measured using micros() or mills().
// value: timeout value.
#define timeout(start_time, current_time, value) (current_time - start_time) > value ? true : false

// Define types of sensors.
#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21

class DHT {
	public:
		DHT(uint8_t pin, uint8_t type);
		void begin(void);
		float readTemperature(bool S=false, bool force=false);
		float convertCtoF(float);
		float convertFtoC(float);
		float computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit=true);
		float readHumidity(bool force=false);
		boolean read(bool force=false);
		inline bool pin_read() __attribute__((always_inline));

	private:
		uint8_t data[5];
		uint8_t _pin, _type, _init_pulse_length;
		#ifdef __AVR
		// Use direct GPIO access on an 8-bit AVR so keep track of the port and bitmask
		// for the digital pin connected to the DHT.  Other platforms will use digitalRead.
		uint8_t _bit, _port;
		#endif
		uint32_t _lastreadtime, _maxcycles;
		bool _lastresult;
};

#endif