#ifndef INCLUDE_VFDISPLAY_H_
#define INCLUDE_VFDISPLAY_H_
#include <SmingCore.h>

class vfdDisplay
{
public:
	static void init();

	static void clear();
	static void show(String val);

    static void showNextEvent(time_t timeNow, time_t timeEvent, String val);
};

#endif /*INCLUDE_VFDISPLAY_H_*/
