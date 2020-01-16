## Description

An ESP8266 device that authenticates with a Microsoft Outlook account with User OAuth 2.0 mechamism to download and display the next meeting location on a serial display device.  This program is based on the SmingHub/Sming framework.

The display device used in test is 03702-029-13120.  A close datasheet is S03701-13-016 for this device.  The display device is on Serial1 connection which has Tx only.

Using the device as it is configured involves a 'boot' stage and then a normal operation stage.

In boot stage, the display will show 'WIFI ...'.  Credentials for WIFI are entered via the Serial connection (USB connector).  From a terminal at 115200 bits/sec, hit enter and parameters are shown :  ssis, pwd, secret, host, and code.

* ssis - WIFI SSIS to connect to
* pwd - password for wifi connection
* secret - is a secret generated from Microsoft OAuth 2.0 services (see links below)
* host - a host name that is also reqired in OAuth 2.0 configuration.  This hostname is suffixed with '.local'.  example:  https://myvfddevice.local
* code - you don't have to enter a value here, it is obtained and saved here from login process

With the Wifi connected,on the serial interface, the host IP will be shown when the WIFI connects and the display will show 'Hostname <host>'.  Navigate a web browser to that IP (or <hostname>.local if mDNS is working). In that interface, provide the tennent ID and the client ID (also obtained from OAuth 2.0 configuration).  Press Login button to login.

The display now shows 'Login ...'.  After login is complete, the next page will attempt to redirect your browser to the device -- this is where host parameter above is important.  In the example above, it will be https://myvfddevice.local/...?code=...

NOTE: HTTPS host AND mDNS needs to work on the ESP8266 device for this to work cleanly.  Both are not working.  A) HTTPS takes too much memory (I have not tested that very carefully), B) mDNS needs ESP8266 Espressif SDK to work but due to memory constraints ENABLE_CUSTOM_LWIP must be enabled which means no mDNS because Sming uses the Espressif functions for mDNS.

As a hack, after login, simply edit the URL to be 'http://..' and the local IP to your host.  This will get around this limitation, all that is needed is that code is written to the configuration file.

After code is written, the web page will refresh.  From there, select 'Refresh' button.  The display will now show 'Polled calendar ...'.  The display should show your next meeting location shortly!

In the first character space of the display, this position indicates the time to your meeting.  An empty space indicates a meeting in over 8 hours. A solid block indicates over 60 minutes to under 8 hours.  A 3-bar symbol for 15 to 30 minutes.  A 2-bar symbol for 10-15 minutes. A 1-bar symbol for 5-10 minutes. And, an astrisk for under 5 minutes and currently under way.

Once the login is performed and the token obtained, the login screen is no longer needed.  The token lasts for an hour, and the refresh token is used to refresh it.  You may have to login again if configuration changes etc. or the device is disconnected for some time and the refresh token times out.

## Microsoft OAuth 2.0 background and how-to

https://docs.microsoft.com/en-ca/azure/active-directory/develop/authentication-scenarios

https://docs.microsoft.com/en-ca/azure/active-directory/develop/v2-oauth2-device-code

https://docs.microsoft.com/en-ca/azure/active-directory/develop/v2-oauth2-auth-code-flow#refresh-the-access-token

