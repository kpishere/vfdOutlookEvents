#include <SmingCore.h>
#include "SerialReadLine.h"
#include "Configuration.h"
#include "webserver.h"
#include "vfdisplay.h"

#define DEVICE_TIMEZONE (-5.0)

DEFINE_FSTR_LOCAL(ssidCmd,"ssid ");
DEFINE_FSTR_LOCAL(pwdCmd,"pwd ");
DEFINE_FSTR_LOCAL(secretCmd,"secret ");
DEFINE_FSTR_LOCAL(hostCmd,"host ");
DEFINE_FSTR_LOCAL(codeCmd,"code ");
DEFINE_FSTR_LOCAL(listCmd,"list");
DEFINE_FSTR_LOCAL(restartCmd,"restart");

SerialReadLine serialReadline;
NtpClient *ntp;

// Will be called when WiFi station network scan was completed
void listNetworks(bool succeeded, BssList& list)
{
    if(!succeeded) {
        Serial.println(_F("Failed to scan networks"));
        return;
    }
    
    for(unsigned i = 0; i < list.count(); i++) {
        Serial.print(_F("\tWiFi: "));
        Serial.print(list[i].ssid);
        Serial.print(", ");
        Serial.print(list[i].getAuthorizationMethodName());
        if(list[i].hidden) {
            Serial.print(_F(" (hidden)"));
        }
        Serial.println();
    }
}

void handleCommand(const String& command)
{
    if(command.startsWith(ssidCmd)) {
        ActiveConfig.ssid = command.substring(ssidCmd.length());
        saveConfig(ActiveConfig);
    } else if(command.startsWith(pwdCmd)) {
        ActiveConfig.pwd = command.substring(pwdCmd.length());
        saveConfig(ActiveConfig);
    } else if(command.startsWith(secretCmd)) {
        ActiveConfig.secret = command.substring(secretCmd.length());
        saveConfig(ActiveConfig);
    } else if(command.startsWith(hostCmd)) {
        ActiveConfig.host = command.substring(hostCmd.length());
        saveConfig(ActiveConfig);
    } else if(command.startsWith(codeCmd)) {
        ActiveConfig.code = command.substring(codeCmd.length());
        saveConfig(ActiveConfig);
    } else if(command.startsWith(listCmd)) {
        WifiStation.startScan(listNetworks);
    } else if(command.startsWith(restartCmd)) {
        System.restart();
    } else {
        Serial.print(_F("Free size (handleCommand): "));
        Serial.println(system_get_free_heap_size());

        Serial.printf(_F("Commands are 'ssid <ssid>', 'pwd <pwd>', 'secret <secret>', 'host <host>', 'code <code>', 'list', 'restart'\r\n"), command.c_str());
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
    
    vfdDisplay::show(F("Hostname ") + ActiveConfig.host);
}

// Will be called when WiFi station was disconnected
void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
    // The different reason codes can be found in user_interface.h. in your SDK.
    Serial.print(_F("Disconnected from \""));
    Serial.print(ssid);
    Serial.print(_F("\", reason: "));
    Serial.println(WifiEvents.getDisconnectReasonDesc(reason));
    
    System.restart(500);
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
    vfdDisplay::show(F("Wifi . . ."));

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
    
    // Optional: Print details of any incoming probe requests
    WifiEvents.onAccessPointProbeReqRecved([](int rssi, MacAddress mac) {
        Serial.print(_F("Probe request: RSSI = "));
        Serial.print(rssi);
        Serial.print(_F(", mac = "));
        Serial.println(mac);
    });

    // Set callback that should be triggered if we are disconnected or connection attempt failed
    WifiEvents.onStationDisconnect(connectFail);
}
