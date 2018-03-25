/* DHT library

MIT license
written by Adafruit Industries
*/

#include "DHT.h"

DHT::DHT(uint8_t pin, uint8_t type) {
	_pin = pin;
	_type = type;
	#ifdef __AVR
	_bit = digitalPinToBitMask(pin);
	_port = digitalPinToPort(pin);
	#endif
}

void DHT::begin(void) {
	// set up the pins!
	pinMode(_pin, INPUT_PULLUP);
	// Using this value makes sure that millis() - lastreadtime will be
	// >= MIN_INTERVAL right away. Note that this assignment wraps around,
	// but so will the subtraction.
	_lastreadtime = -MIN_INTERVAL;
}

//boolean S == Scale.  True == Fahrenheit; False == Celcius
float DHT::readTemperature(bool S, bool force) {
	float f = NAN;

	if (read(force)) {
		switch (_type) {
			case DHT11:
			f = data[2];
			if(S) {
				f = convertCtoF(f);
			}
			break;
			case DHT22:
			case DHT21:
			f = data[2] & 0x7F;
			f *= 256;
			f += data[3];
			f *= 0.1;
			if (data[2] & 0x80) {
				f *= -1;
			}
			if(S) {
				f = convertCtoF(f);
			}
			break;
		}
	}
	return f;
}

float DHT::convertCtoF(float c) {
	return c * 1.8 + 32;
}

float DHT::convertFtoC(float f) {
	return (f - 32) * 0.55555;
}

float DHT::readHumidity(bool force) {
	float f = NAN;
	if (read()) {
		switch (_type) {
			case DHT11:
			f = data[0];
			break;
			case DHT22:
			case DHT21:
			f = data[0];
			f *= 256;
			f += data[1];
			f *= 0.1;
			break;
		}
	}
	return f;
}

//boolean isFahrenheit: True == Fahrenheit; False == Celcius
float DHT::computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit) {
	// Using both Rothfusz and Steadman's equations
	// http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
	float hi;

	if (!isFahrenheit)
	temperature = convertCtoF(temperature);

	hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (percentHumidity * 0.094));

	if (hi > 79) {
		hi = -42.379 +
		2.04901523 * temperature +
		10.14333127 * percentHumidity +
		-0.22475541 * temperature*percentHumidity +
		-0.00683783 * pow(temperature, 2) +
		-0.05481717 * pow(percentHumidity, 2) +
		0.00122874 * pow(temperature, 2) * percentHumidity +
		0.00085282 * temperature*pow(percentHumidity, 2) +
		-0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

		if((percentHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0))
		hi -= ((13.0 - percentHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

		else if((percentHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0))
		hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
	}

	return isFahrenheit ? hi : convertFtoC(hi);
}

boolean DHT::read(bool force) {
	
	/*
	 * Method responsible for reading the sensor.
	 */

	uint32_t bit_start_time;
	int32_t bit_time;

	// Check if sensor was read less than two seconds ago and return early
	// to use last reading.
	uint32_t current_time = micros();
	if (!force && ((current_time - _lastreadtime) < 2000000)) {
		return _lastresult; // return last correct measurement
	}
	_lastreadtime = current_time;

	// Reset 40 bits of received data to zero.
	data[0] = data[1] = data[2] = data[3] = data[4] = 0;

	// Send start signal. See DHT data sheet for full signal diagram:
	// http://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf

	// Go into high impedance state to let pull-up raise data line level and
	// start the reading process.
	
	digitalWrite(_pin, HIGH);
	delayMicroseconds(100);

	// First set data line low for 1 millisecond.
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, LOW);
	delay(1);

	// End the start signal by setting data line high for 40 microseconds.
	digitalWrite(_pin, HIGH);
	pinMode(_pin, INPUT_PULLUP);
	delayMicroseconds(40);
	
	// HIGH - 20-40us, host signal
    current_time = micros();
	while (pin_read() == HIGH) {
		if (timeout(current_time, micros(), 100)) {
			DEBUG_PRINTLN(F("Read timeout."))
			_lastresult = false;
			return _lastresult;
		}
	}

	// LOW - 80us, sensor signal
    current_time = micros();
	while (pin_read() == LOW) {
		if (timeout(current_time, micros(), 100)) {
			DEBUG_PRINTLN(F("Read timeout."))
			_lastresult = false;
			return _lastresult;
		}
	}
	
	// HIGH - 80us, sensor signal
    current_time = micros();
	while (pin_read() == HIGH) {
		if (timeout(current_time, micros(), 100)) {
			DEBUG_PRINTLN(F("Read timeout."))
			_lastresult = false;
			return _lastresult;
		}
	}
		
	for (uint8_t bit_cnt = 0; bit_cnt < 40; bit_cnt++) {
			
		// Low state ('0') initializing 1 bit transmission.
		// Should last about 50 us, sensor signal
        bit_start_time = current_time = micros();
		do {
            
            if (timeout(current_time, bit_start_time, 100)) {
				DEBUG_PRINTLN(F("Read timeout."))
				_lastresult = false;
				return _lastresult;
			}
            
            bit_start_time = micros();

		} while (pin_read() == LOW); // LOW

        // The high state depends on transmitted value.
        // If sensor outputs '0' then the hight state lasts 26-28us.
        // If the sensor outputs '1' then the high state lasts 70us.
        // The timeout macro is intentionally not used.
		do {
			
			bit_time = micros() - bit_start_time;
				
			// cope with the micros() function overflow
			if (bit_time < 0) {
				bit_time += 0x100000000;
			}
				
			// timeout
			if (bit_time > 100) {
				DEBUG_PRINTLN(F("Time difference too big."))
				_lastresult = false;
				return _lastresult;
			}

		} while (pin_read() == HIGH); // HIGH
		
		// analyze the bit timing
		data[bit_cnt / 8] <<= 1;
		if (bit_time >= 50) {
			data[bit_cnt / 8] |= 1;
		}

	}

	DEBUG_PRINTLN(F("Received:"));
	DEBUG_PRINT(data[0], HEX); DEBUG_PRINT(F(", "));
	DEBUG_PRINT(data[1], HEX); DEBUG_PRINT(F(", "));
	DEBUG_PRINT(data[2], HEX); DEBUG_PRINT(F(", "));
	DEBUG_PRINT(data[3], HEX); DEBUG_PRINT(F(", "));
	DEBUG_PRINT(data[4], HEX); DEBUG_PRINT(F(" =? "));
	DEBUG_PRINTLN((data[0] + data[1] + data[2] + data[3]) & 0xFF, HEX);

	// Check that the checksum matches.
	if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
		_lastresult = true;
		return _lastresult;
	} else {
		DEBUG_PRINTLN(F("Checksum failure!"));
		_lastresult = false;
		return _lastresult;
	}
}

// local pin read function.
inline bool DHT::pin_read() {
	#ifdef __AVR
        // On AVR platforms use direct GPIO port access as it's much faster and better
        // for catching pulses that are 10's of microseconds in length:
		return (*portInputRegister(_port) & _bit) ? HIGH : LOW;
	#else
        // Otherwise fall back to using digitalRead (this seems to be necessary on ESP8266
        // right now, perhaps bugs in direct port access functions?).
		return digitalRead(_pin);
	#endif
}

