#include <SmingCore.h>
#include "SerialReadLine.h"
#include "Configuration.h"
#include "webserver.h"
#include "vfdisplay.h"

#define DEVICE_TIMEZONE (-5.0)

DEFINE_FSTR(ssidCmd,"ssid ");
DEFINE_FSTR(pwdCmd,"pwd ");
DEFINE_FSTR(secretCmd,"secret ");
DEFINE_FSTR(hostCmd,"host ");
DEFINE_FSTR(codeCmd,"code ");

SerialReadLine serialReadline;
NtpClient *ntp;

void handleCommand(const String& command)
{
    if(command.startsWith(ssidCmd)) {
        ActiveConfig.ssid = String(command.substring(ssidCmd.length()));
        saveConfig(ActiveConfig);
    } else if(command.startsWith(pwdCmd)) {
        ActiveConfig.pwd = String(command.substring(pwdCmd.length()));
        saveConfig(ActiveConfig);
    } else if(command.startsWith(secretCmd)) {
        ActiveConfig.secret = String(command.substring(secretCmd.length()));
        saveConfig(ActiveConfig);
    } else if(command.startsWith(hostCmd)) {
        ActiveConfig.host = String(command.substring(hostCmd.length()));
        saveConfig(ActiveConfig);
    } else if(command.startsWith(codeCmd)) {
        ActiveConfig.code = String(command.substring(codeCmd.length()));
        saveConfig(ActiveConfig);
    } else {
        Serial.print("Free size (handleCommand): ");
        Serial.println(system_get_free_heap_size());

        Serial.printf(_F("Commands are 'ssid <ssid>', 'pwd <pwd>', 'secret <secret>', 'host <host>', 'code <code>'\r\n"), command.c_str());
    }
}

// Will be called when WiFi station becomes fully operational
void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
    vfdDisplay::clear();

    startWebServer();
    startmDNS();
    if(ntp != NULL) delete ntp;
    ntp = new NtpClient(_F("pool.ntp.org"), 300);
    
    vfdDisplay::show(String(_F("Hostname ")) + ActiveConfig.host);
}

void init()
{
    ntp = NULL;
    // Command and debug serial port
	Serial.setTxBufferSize(128);
    Serial.setRxBufferSize(1024);
	Serial.setTxWait(false);
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
    Serial.systemDebugOutput(true);

    spiffs_mount();
    delay(2000);

    vfdDisplay::init();
    vfdDisplay::clear();
    vfdDisplay::show("Wifi . . .");

    ActiveConfig = loadConfig();
    
    SystemClock.setTimeZone( DEVICE_TIMEZONE );
    
    serialReadline.begin(Serial);
    serialReadline.onCommand(handleCommand);

    // Configure wifi
    WifiStation.enable(true);
    WifiStation.config(ActiveConfig.ssid, ActiveConfig.pwd);
    WifiAccessPoint.enable(false);
    
    // Run our method when station was connected to AP
    WifiEvents.onStationGotIP(gotIP);
}
