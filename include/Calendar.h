#ifndef INCLUDE_CALENDAR_H
#define INCLUDE_CALENDAR_H

#include <SmingCore.h>

class Calendar {
public:
    static void getCalendar();
    static void getCalendarIn(unsigned ms);

    static void onCodeSave(HttpParams& parms);
    static void getToken(boolean isRefresh = false);    
    static void getAuthorization(String filename_login);
};

#endif
