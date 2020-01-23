/*
 * SerialReadLine.cpp
 */

#include "SerialReadLine.h"

void SerialReadLine::begin(HardwareSerial& serial)
{
	this->serial = &serial;
	serial.onDataReceived(StreamDataReceivedDelegate(&SerialReadLine::onData, this));
	debug_d("hwsDelegateDemo instantiated, waiting for data");
}

void SerialReadLine::onData(Stream& stream, char arrivedChar, unsigned short availableCharsCount)
{
	/*
    serial->print(_F("Class Delegate Demo Time = "));
    serial->print(micros());
    serial->print(_F(" char = 0x"));
    serial->print(arrivedChar, HEX); // char hex code
    serial->print(_F(" available = "));
    serial->println(availableCharsCount);
    */

	// Error detection
	unsigned status = serial->getStatus();
	if(status != 0) {
		if(bitRead(status, eSERS_Overflow)) {
			serial->println(_F("** RECEIVE OVERFLOW **"));
		}
		if(bitRead(status, eSERS_BreakDetected)) {
			serial->println(_F("** BREAK DETECTED **"));
		}
		if(bitRead(status, eSERS_FramingError)) {
			serial->println(_F("** FRAMING ERROR **"));
		}
		if(bitRead(status, eSERS_ParityError)) {
			serial->println(_F("** PARITY ERROR **"));
		}
		// Discard what is likely to be garbage
		serial->clear(SERIAL_RX_ONLY);
		return;
	}

	numCallback++;

	if(arrivedChar == '\n') // Lets show data!
	{
		debug_d(_F("<New line received>"));
		// Read the string into a line
		String line;
		line.reserve(availableCharsCount);
		while(availableCharsCount--) {
			char cur = stream.read();
			charReceived++;
			//serial->print(cur);
			if(cur != '\n' && cur != '\r') {
				line += cur;
			}
		}
		//serial->println();

		if(callback) {
			callback(line);
		}
	}
}
