#ifndef INCLUDE_SERIALREADLINE_H_
#define INCLUDE_SERIALREADLINE_H_

#include <SmingCore.h>

typedef Delegate<void(const String& command)> CommandCallbackFunction;

//*** Example of class callback processing
class SerialReadLine
{
public:
    void begin(HardwareSerial& serial);
    void onData(Stream& stream, char arrivedChar, unsigned short availableCharsCount);
    
    void onCommand(CommandCallbackFunction callback)
    {
        this->callback = callback;
    }
    
private:
    HardwareSerial* serial = nullptr;
    unsigned charReceived = 0;
    unsigned numCallback = 0;
    bool useRxFlag = true;
    CommandCallbackFunction callback = nullptr;
};

#endif /* INCLUDE_SERIALREADLINE_H_ */

