
#include "vfdisplay.h"
/*
 ** Use to wake up display, prints nothing
 00h Null 4.6
 
 ** Only for parallel interface
 01h Prepare to Read Display Identification 4.4
 02h Prepare to Read Software Check sum 4.4
 03h Prepare to Read Cursor Location 4.4
 04h Prepare to Read Data at Cursor Location 4.4
 05h Prepare to Read Data at Cursor Location and Increment 4.4
 06h Unassigned
 
 07h Bell/Alarm Output 4.6
 08h Backspace Cursor 4.2
 09h Advance Cursor 4.2 ** ASCII Horizontal tab
 0Ah Line Feed 4.2
 0Bh Blink Block Cursor 4.2 ** ASCII Vertical tab
 0Ch Underbar Cursor / End Blink 4.2 ** ASCII Form feed
 0Dh Carriage Return 4.2
 0Eh Cursor Off 4.2 ** ASCII Shift out
 0Fh Cursor On 4.2 ** ASCII Shift in
 10h Scroll Line Lock 4.6 ** ASCII Data link escape
 11h Set Vertical Scroll Mode 4.3 ** ASCII XON
 12h Unassigned ** ASCII Device control 2
 13h Set Horizontal Scroll Mode 4.3 ** ASCII XOFF
 
 14h Software Reset 4.6 ** ASCII Device control 4
 15h Clear Display and Home Cursor 4.2 ** ASCII NAK
 16h Home Cursor 4.2 ** ASCII Synch Idle
 17h Set Data Bit 7 High 4.6 ** ASCII EOT
 18h Begin User Defined Character 4.3 ** ASCII Cancel
 19h ** Set Address Bit 0 High 4.6 ** ASCII End of Medium
 1Ah Cursor up One Line 4.2 ** ASCII Substitute
 1Bh Move Cursor to Designated Location + second byte for N postion 4.2 ** ASCII ESC
 1Ch Select European Character Set 4.3 ** ASCII File separator
 1Dh Select Katakana Character Set 4.3 ** ASCII Group separator
 1Eh Select Cyrillic 4.3 ** ASCII Record separator
 1Fh Select Hebrew Character Set 4.3 ** ASCII Unit separator
 
 20-7F Map as ASCII Printable [space] thru to [?]
 
 20-3F With comma is 80-9F              0x20 - 0010 0000 -> 0x80 - 1000 0000
 20-3F With decimal point is A0-Bf                          0xA0 - 1010 0000
 40-4F With comma is E0-FF              0x40 - 0100 0000    0xE0 - 1110 0000
 40-5F With decimal point is C0-Df                          0xC0 - 1100 0000
 
 ** With A0 bit High, the following codes apply
 30h Set Display Screen or Column Brightness Level
 31h Begin Blinking Character(s)
 32h End Blinking Character(s)
 33h Blank Display Screen
 34h Unblank Display Screen
 +35h Comma/Period/Triangle Function
 36h Erase Line data With End Blink
 37h Set Carriage Return and Line Feed Definitions 38h Underbar On
 39h Underbar Off
 3Ah Select Right to Left Data Entry
 3Bh Select Left to Right Data Entry
 3Ch Screen Saver On
 3Dh Screen Saver Off
 3Eh Execute Self–test
 3Fh Terminate Self–test
 */
HardwareSerial Serial1(UART_ID_1);

void vfdDisplay::init()
{
	// Initialise and prepare the second serial port for display
	Serial1.begin(19200, SERIAL_8N2);
	Serial1.systemDebugOutput(false);
	Serial1.setTxBufferSize(128);
	Serial1.setRxBufferSize(0);
	Serial1.setTxWait(false);
	delay(200);
}

void vfdDisplay::clear()
{
	Serial1.print('\x15'); // Clear and reset display
	Serial1.print('\x0E'); // disable curser flash
}

void vfdDisplay::show(String val)
{
	val.toUpperCase(); // Lower case don't print with punctuation (and look like upper case)
	String vfdStr;
	for(char c : val) {
		// Directly printable characters only
		if(('\x20' <= c && c <= '\x60') || '\x7B' <= c && c <= '\x7E') {
			vfdStr += c;
		}
	}

	// TODO: Parse HTML here for character control codes supported
	if(vfdStr.length() <= 1)
		return;

	// Search and replace supported tokens
	for(int i = 1; i < vfdStr.length(); i++) {
		char cv = vfdStr[i - 1];
		switch(vfdStr[i]) {
		case ',':
			if('\x20' <= cv && cv <= '\x3F')
				cv += 0x60;
			else if('\x40' <= cv && cv <= '\x5F')
				cv += 0xA0;
			break;
		case '.':
			if('\x20' <= cv && cv <= '\x5F')
				cv += 0x80;
			break;
		}
		vfdStr[i - 1] = cv;
	}
	vfdStr.replace(",", "");
	vfdStr.replace(".", "");

	Serial1.print(vfdStr);
}

void vfdDisplay::showNextEvent(time_t timeNow, time_t timeEvent, String val)
{
    bool inFuture = (timeEvent > timeNow);
    int minsToEvent = (inFuture ? timeEvent - timeNow : -1 * (timeNow - timeEvent)) / SECS_PER_MIN;
	Serial.print(F("mins: "));
	Serial.println(minsToEvent);

	vfdDisplay::clear();
	String indicator;
	if(minsToEvent > 60 * 8 /*h*/) {
		indicator = ' ';
	} else {
		if(minsToEvent > 30) {
			indicator = F("\x0B\x7F\x0C"); // [*]
		} else {
			if(minsToEvent > 15) {
				indicator = '\x23'; // ==
			} else {
				if(minsToEvent > 10) {
					indicator = '\x3A'; // =
				} else {
					if(minsToEvent > 5) {
						indicator = '\x5F'; // _
					} else {
						indicator = F("\x0B\x2A\x0C"); // *
					}
				}
			}
		}
	}
	vfdDisplay::clear();
	Serial1.print(indicator);
	vfdDisplay::show(val);
}
