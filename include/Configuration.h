#ifndef INCLUDE_CONFIGURATION_H_
#define INCLUDE_CONFIGURATION_H_

#include <SmingCore.h>

struct Configuration {
	Configuration()
	{
		ssid = "";
		pwd = "";
		host = "esp8266";
		tennentId = "";
		clientId = "";
		code = "";
		secret = "";
	}

	// network
	String ssid;
	String pwd;
	String host;

	// OAuthUser
	String tennentId;
	String clientId;
	String code;
	String secret;
};

Configuration loadConfig();
void saveConfig(Configuration& cfg);

extern Configuration ActiveConfig;

#endif /* INCLUDE_CONFIGURATION_H_ */
