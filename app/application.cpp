#include <SmingCore.h>
#include "Data/Stream/FileStream.h"
#include "SerialReadLine.h"
#include "Configuration.h"
#include "webserver.h"
#include "vfdisplay.h"

#define DEVICE_TIMEZONE (-5.0)

DEFINE_FSTR_LOCAL(ssidCmd, "ssid ");
DEFINE_FSTR_LOCAL(pwdCmd, "pwd ");
DEFINE_FSTR_LOCAL(secretCmd, "secret ");
DEFINE_FSTR_LOCAL(hostCmd, "host ");
DEFINE_FSTR_LOCAL(codeCmd, "code ");
DEFINE_FSTR_LOCAL(listCmd, "list");
DEFINE_FSTR_LOCAL(dirCmd, "dir");
DEFINE_FSTR_LOCAL(dirCat, "cat");
DEFINE_FSTR_LOCAL(restartCmd, "restart");

SerialReadLine serialReadline;
NtpClient* ntp;

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
void listFiles() {
    Vector<String> fList = fileList();
    Serial.println("\r\nFiles:");
    for (unsigned i = 0; i < fList.size(); i++) {
        Serial.println(fList[i]);
    }
}
void printFile(const String& fname) {
    FileStream *fileStream = new FileStream();
    char b[1024];
    int readBytes = 0;
    if(fileStream->open(fname)) {
        readBytes = fileStream->readMemoryBlock((char *)b,1024);
        Serial.write((char *)b,readBytes);
        Serial.println("<eof>");
    } else {
        Serial.println("file not found.");
    }
    delete fileStream;
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
    } else if(command.startsWith(dirCmd)) {
        listFiles();
    } else if(command.startsWith(dirCat)) {
        String fname = command.substring(dirCat.size());
        printFile(fname);
    } else if(command.startsWith(restartCmd)) {
		System.restart();
	} else {
		Serial.print(_F("Free size (handleCommand): "));
		Serial.println(system_get_free_heap_size());

		Serial.println(_F("Commands are: \r\n"
						  "ssid <ssid>\r\n"
						  "pwd <pwd>\r\n"
						  "secret <secret>\r\n"
						  "host <host>\r\n"
						  "code <code>\r\n"
						  "list\r\n"
                          "dir\r\n"
                          "cat\r\n"
						  "restart"));
	}
}

// Will be called when WiFi station becomes fully operational
void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	vfdDisplay::clear();

	startWebServer();
	startmDNS();
	if(ntp != NULL)
		delete ntp;
	ntp = new NtpClient(nullptr, 300);

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

	System.restart(30000);
}
void startWiFi() {
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

void init()
{
	ntp = NULL;
	// Command and debug serial port
	Serial.setTxBufferSize(1024);
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

	SystemClock.setTimeZone(DEVICE_TIMEZONE);

	serialReadline.begin(Serial);
	serialReadline.onCommand(handleCommand);
    
    startWiFi();
}
