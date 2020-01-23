#include "vfdisplay.h"
#include <JsonObjectStream.h>
#include "Calendar.h"
#include "Configuration.h"
#include <time.h>

const uint16_t CalendarJsonBufferSize = 4096;

void displayUpdate();

const int pollTime_ms =  1 * 59 * 1000; /* min * s/min * ms/s */;
const int pollTime_qry =  14; /* queries per display refreshes */;
int procTimer_multple = 0;
Timer procTimer;
Timer deadTimer;

HttpClient httpClient;

DEFINE_FSTR_LOCAL(filename_token,"token.json");
DEFINE_FSTR_LOCAL(filename_update,"update.json");
DEFINE_FSTR_LOCAL(lastQuery_json,"query.json");
DEFINE_FSTR_LOCAL(code_token,"code");

/*
 * Configure SSL features and trust
 */
void onSslInit(Ssl::Session& session, HttpRequest& request) {
	// Microsoft IT TLS CA 4
	if(request.uri.Host == _F("login.microsoftonline.com")) {
		static const Ssl::Fingerprint::Cert::Sha1 fpCertLogin PROGMEM = {
			0x54, 0x52, 0x80, 0x88, 0x47, 0xfe, 0x67, 0x4a, 0xba, 0x8c,
			0xb6, 0x45, 0x7b, 0xd3, 0x37, 0x62, 0x7c, 0x2c, 0x56, 0xa0,
		};
		static const Ssl::Fingerprint::Pki::Sha256 fpPkiLogin PROGMEM = {
			62,   0x91, 0xaf, 0x46, 0xc3, 0x69, 0x17, 0x6a, 0x6e, 0x17, 0x6e, 0xa4, 0x6d, 0x9e, 0x2f, 0xdd,
			0x57, 0x56, 0x3a, 0xcd, 0x4b, 0x0c, 0x8d, 0xb9, 0x79, 0x0f, 0x1d, 0xfe, 0x9b, 0xad, 0x86, 0x0d,
		};
		session.validators.pin(fpCertLogin);
		session.validators.pin(fpPkiLogin);
	} else {
		if(request.uri.Host == _F("graph.microsoft.com")) {
			static const Ssl::Fingerprint::Cert::Sha1 fpCertService PROGMEM = {
				0xA2, 0x5F, 0x6E, 0xFE, 0xFD, 0x54, 0xAF, 0xF4, 0x49, 0x16,
				0xEC, 0xDB, 0xD9, 0x8C, 0xE8, 0x19, 0xBD, 0xFB, 0x64, 0x54,
			};
			static const Ssl::Fingerprint::Pki::Sha256 fpPkiService PROGMEM = {
				0x73, 0x0B, 0x3A, 0x91, 0xF7, 0x82, 0xDE, 0x81, 0x7C, 0x7E, 0x8D, 0xB9, 0xCA, 0x84, 0x6D, 0xC9,
				0xBD, 0xE1, 0x41, 0xA6, 0x62, 0xE6, 0x06, 0x12, 0xCC, 0xF3, 0x6E, 0x14, 0xA9, 0x93, 0xAF, 0x5B,
			};
			session.validators.pin(fpCertService);
			session.validators.pin(fpPkiService);
		} else {
			Serial.print(_F("No trust for "));
			Serial.println(request.uri.Host);
		}
	}

	// We're using fingerprints, so don't attempt to validate full certificate
    session.options.verifyLater = true;
    
    // Explicity set as a reminder (this is the default and many servers only send 16K blocks regardless of client request)
    session.maxBufferSize = Ssl::MaxBufferSize::K16;
}

// 2020-01-13T14:20:00.0000000 or 2020-01-13T14:20:00Z or 2020-01-13
DateTime parseISO8602(String datetime) {
    DateTime tm;
    short loc[] = {0, 5, 8, 11, 14, 17};
    char buffer[20];
    int v, yr = 1970, mnth = 01, d = 01, h = 0, m = 0, s = 0, len;
    len = datetime.length();
    len = (len<20?len:20);
    
    memcpy(buffer, datetime.c_str(), len);
    for(int i=0; i<len; i++) {
        if(buffer[i] == '-' || buffer[i] == 'T' || buffer[i] == ':' || buffer[i] == '.' || buffer[i] == 'Z') {
            buffer[i] = 0;
        }
    }
    for(short i=0; i<((len>9) * 3 + (len>18) * 3)  ; i++) {
        v = String(&buffer[loc[i]]).toInt();
        switch(i) {
            case 0: yr = v; break;
            case 1: mnth = v-1; break;
            case 2: d = v; break;
            case 3: h = v; break;
            case 4: m = v; break;
            case 5: s = v; break;
        }
    }
    tm.setTime(s, m, h, d, mnth, yr);
    return tm;
}
void getTokenRefresh() {
    httpClient.cleanup();
    Calendar::getToken(true);
}
int getTokenRefreshComplete(HttpConnection& connection, bool success) {
    DynamicJsonDocument doc(CalendarJsonBufferSize);
    
    // If update token received successfully, move it to current token file
    if(success) {
        if(Json::loadFromFile(doc, filename_update)) {
            if(doc.containsKey(_F("access_token"))) {
                doc.clear();
                fileDelete(filename_token);
                fileRename(filename_update, filename_token);
            }
        }
    }
    // Schedule poll event - only called after a token refresh
    procTimer.initializeMs( 120 * 1000, Calendar::getCalendar).startOnce();
    return 0;
}
void noActivityRestart() {
    System.restart();
}
void displayUpdate()
{
    DynamicJsonDocument doc(CalendarJsonBufferSize);
    
    // Update Display
    if(Json::loadFromFile(doc, lastQuery_json)) {
        JsonObject error = doc[_F("error")];
        if(error) {
            Serial.printf("%s %s\n", error[_F("code")].as<const char*>(), error[_F("message")].as<const char*>());
            if(String(_F("InvalidAuthenticationToken")).compareTo( error[_F("code")].as<const char*>() ) == 0 ) {
                procTimer.initializeMs(10 * 1000, getTokenRefresh).startOnce();
                doc.clear();
                return;
            }
        }
        JsonArray valueArray = doc[_F("value")];

        Serial.printf(_F("Calendar items : %d\n"),valueArray.size() );
        
        if(valueArray.size() >0) {
            JsonObject location = valueArray[0][_F("location")];
            JsonObject start = valueArray[0][_F("start")]
            ,   end = valueArray[0][_F("end")];
            DateTime dt_start = parseISO8602( String(start[_F("dateTime")].as<const char*>()) )
                   ,   dt_end = parseISO8602( String(  end[_F("dateTime")].as<const char*>()) );
            Serial.printf(_F("start %s end %s\n")
                          , (const char *)dt_start.toISO8601().c_str(), (const char *)dt_end.toISO8601().c_str()
                          );
            if(dt_end.toUnixTime() - SystemClock.now(eTZ_UTC) > 0) {
                vfdDisplay::showNextEvent( (dt_start.toUnixTime() - SystemClock.now(eTZ_UTC)) / 60 , location[_F("displayName")] );
            } else {
                vfdDisplay::clear();
            }
        } else { // Show time by default
            vfdDisplay::clear();
            //vfdDisplay::show( SystemClock.getSystemTimeString() );
        }
    } else {
    }
    // update dead timer
    procTimer.initializeMs( 3 * pollTime_ms, noActivityRestart ).startOnce();
    // Schedule poll event
    if( procTimer_multple % pollTime_qry == 0) {
        procTimer.initializeMs( pollTime_ms, Calendar::getCalendar ).startOnce();
    } else {
        procTimer.initializeMs( pollTime_ms, displayUpdate ).startOnce();
    }
    procTimer_multple++;
}

int callDisplayUpdate(HttpConnection& connection, bool success) {
    debug_e("calendar update");
    
    //    procTimer.initializeMs(10000, displayUpdate).start();
    displayUpdate();

    return 0;
}



void Calendar::getAuthorization(String filename_login) {
    Url url;
    
    // Configure request for User OAuth 2.0 credential access to a service
    url.Scheme = URI_SCHEME_HTTP_SECURE;
    url.Host = String(_F("login.microsoftonline.com"));
    url.Path = String(_F("/"))+ActiveConfig.tennentId+_F("/oauth2/v2.0/authorize");
    url.Query[F("scope")] = String(_F("offline_access openid profile calendars.read"));
    url.Query[F("response_type")] = String(_F("code"));
    url.Query[F("client_id")] = ActiveConfig.clientId;
    url.Query[F("redirect_uri")] = String(_F("https://")+ ActiveConfig.host + _F(".local/register"));
    url.Query[F("response_mode")] = String(_F("query"));
    
    HttpRequest* request1 = new HttpRequest(url.toString());
    request1->onSslInit(onSslInit);
    request1->setMethod(HTTP_GET);
    
    // Send request using our httpClient, saves response locally
    FileStream* fileStream = new FileStream;
    if(fileStream && fileStream->open(filename_login, eFO_CreateNewAlways | eFO_ReadWrite)) {
        request1->setResponseStream(fileStream); // saving response here
        httpClient.send(request1);
    }
    vfdDisplay::clear();
    vfdDisplay::show("Login . . .");
}

/*
 * If one could run an HTTPS service, this could be called and code value set here.
 * Alternately, use serial interface to set 'code' value.
 */
void Calendar::onCodeSave(HttpParams& parms) {
    //Serial.printf("\nRegister [ URL: %s ]", parms[code_token].c_str());
    ActiveConfig.code = parms[code_token].c_str();
    saveConfig(ActiveConfig);

    /* This would be where the app is 'connected' as tokens expire and
     we must re-use this code to update token until permissions of app are revoked.
     */
    vfdDisplay::clear();
    vfdDisplay::show("Connected . . .");
}

void Calendar::getToken(boolean isRefresh) {
    Url url;
    HttpParams formBody;
    
    /*
     $body = "client_id=$clientID&client_secret=$clientSecret&scope=$scopes&grant_type=authorization_code&code=$code&redirect_uri=$redirectUrl"
     #v2.0 token URL
     $tokenUrl = "https://login.microsoftonline.com/common/oauth2/v2.0/token"
     
     $response = Invoke-RestMethod -Method Post -Uri $tokenUrl -Headers @{"Content-Type" = "application/x-www-form-urlencoded"} -Body $body
     */
        
    // Configure request for User OAuth 2.0 credential access to a service
    url.Scheme = URI_SCHEME_HTTP_SECURE;
    url.Host = String(_F("login.microsoftonline.com"));
    url.Path = String(_F("/common/oauth2/v2.0/token"));

    HttpRequest* request1 = new HttpRequest(url.toString());

    formBody[F("client_id")] = ActiveConfig.clientId;
    formBody[F("client_secret")] = ActiveConfig.secret;
    formBody[F("scope")] = String(_F("offline_access openid profile calendars.read"));
    formBody[F("grant_type")] = String( (isRefresh ? _F("refresh_token") : _F("authorization_code")) );
    if(isRefresh) {
        DynamicJsonDocument doc(CalendarJsonBufferSize);
        
        if(Json::loadFromFile(doc, filename_token)) {
            formBody[F("refresh_token")] = String( doc[_F("refresh_token")].as<const char*>() );
        }
    } else {
        formBody[F("code")] = ActiveConfig.code;
    }
    formBody[F("redirect_uri")] = String(_F("https://")+ ActiveConfig.host + _F(".local/register"));
    
    Serial.println(url.toString());
    Serial.print(formBody.toString().substring(1));

    request1->setHeader(F("Content-Type"), F("application/x-www-form-urlencoded"));
    request1->onSslInit(grcSslInit);
    request1->setMethod(HTTP_POST);
    request1->setBody(formBody.toString().substring(1));
    if(isRefresh) {
        request1->onRequestComplete(getTokenRefreshComplete);
    }

    // Send request using our httpClient, saves response locally
    FileStream* fileStream = new FileStream;
    if(fileStream && fileStream->open( (isRefresh ? filename_update : filename_token)
                                      , eFO_CreateNewAlways | eFO_ReadWrite)) {
        request1->setResponseStream(fileStream); // saving response here
        httpClient.send(request1);
    }
}

void Calendar::getCalendarIn(int ms) {
    procTimer.initializeMs( ms, Calendar::getCalendar).startOnce();
}
void Calendar::getCalendar() {
    DynamicJsonDocument doc(CalendarJsonBufferSize);
    Url url;
    DateTime dt_start(SystemClock.now(eTZ_UTC)), dt_end(SystemClock.now(eTZ_UTC) + 60*60*24);

    /*
     
     GET https://graph.microsoft.com/v1.0/me/calendarview?startDateTime=2020-01-12T16%3a35%3a05Z&endDateTime=2020-01-13T16%3a35%3a05Z&$select=Subject,location,start&$orderby=start/dateTime&$top=2
     
     Response body:
     
     {
     "@odata.context": "https://graph.microsoft.com/v1.0/$metadata#users('bb6832fa-cedb-4dc2-b30a-be79bf262987')/calendarView",
     "value": [
     {
     "@odata.etag": "W/\"42ZAiN6iEEeXIepvOtkZ8QACMVLpqA==\"",
     "location": {
     "displayName": "Skype Meeting",
     "locationType": "default",
     "uniqueId": "Skype Meeting",
     "uniqueIdType": "private"
     },
     "start": {
     "dateTime": "2020-01-13T15:00:00.0000000",
     "timeZone": "UTC"
     },
     "subject": "RF smart deployment",
     },
     {
     "@odata.etag": "W/\"42ZAiN6iEEeXIepvOtkZ8QACMVLptg==\"",
     "location": {
     "displayName": "Wherever The Board",
     "locationType": "default",
     "uniqueId": "Wherever The Board",
     "uniqueIdType": "private"
     },
     "start": {
     "dateTime": "2020-01-13T14:20:00.0000000",
     "timeZone": "UTC"
     },
     "subject": "Back of House Stand Up",
     }
     ]
     }
    */
    
    Serial.print( dt_start.toFullDateTimeString()  + "\n");
    if(Json::loadFromFile(doc, filename_token)) {
        Url url;
        
        // Configure request for User OAuth 2.0 credential access to a service
        url.Scheme = URI_SCHEME_HTTP_SECURE;
        url.Host = String(_F("graph.microsoft.com"));
        url.Path = String(_F("/v1.0/me/calendarview"));
        
        url.Query[F("startDateTime")] = dt_start.toISO8601();
        url.Query[F("endDateTime")] = dt_end.toISO8601();
        url.Query[F("$select")] = String(_F("Subject,location,start,end"));
        url.Query[F("$orderby")] = String(_F("start/dateTime"));
        url.Query[F("$top")] = String(_F("2"));
        
        Serial.println(url.toString());
        
        HttpRequest* request1 = new HttpRequest(url.toString());
        request1->onSslInit(grcSslInit);
        request1->setMethod(HTTP_GET);
        request1->setHeader("Authorization", doc["access_token"] );
        request1->onRequestComplete(callDisplayUpdate);

        // Send request using our httpClient, saves response locally
        FileStream* fileStream = new FileStream;
        if(fileStream && fileStream->open(lastQuery_json, eFO_CreateNewAlways | eFO_ReadWrite)) {
            request1->setResponseStream(fileStream); // saving response here
            httpClient.send(request1);
            
            vfdDisplay::clear();
            vfdDisplay::show("Polled calendar . . .");
        }
    }
}
