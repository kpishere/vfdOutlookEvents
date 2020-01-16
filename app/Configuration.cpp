#include <JsonObjectStream.h>
#include "Configuration.h"

const uint16_t ConfigJsonBufferSize = 2048;

DEFINE_FSTR(configFileName,".config.conf");  // leading period for files un-downloadable by webserver
DEFINE_FSTR(networkName,"network");
DEFINE_FSTR(ssidName,"StaSSID");
DEFINE_FSTR(pwdName,"StaPassword");
DEFINE_FSTR(hostName,"HostName");

DEFINE_FSTR(tennentName,"tennentid");
DEFINE_FSTR(clientIdName,"clientid");
DEFINE_FSTR(codeName,"code");
DEFINE_FSTR(clientSecretName,"secret");

const char endOfLine = '\n';

Configuration ActiveConfig;

Configuration loadConfig()
{
    DynamicJsonDocument doc(ConfigJsonBufferSize);
    Configuration cfg;
    
    if(Json::loadFromFile(doc, configFileName)) {
        //debug_i("Load values: ");
        //Json::serialize(doc, Serial, Json::Pretty); // For debugging

        JsonObject network = doc[networkName];
        cfg.ssid = String(network[ssidName].as<const char*>());
        cfg.pwd = String(network[pwdName].as<const char*>());
        cfg.host = String(network[hostName].as<const char*>());
        
        cfg.tennentId=String(network[tennentName].as<const char*>());
        cfg.clientId=String(network[clientIdName].as<const char*>());
        cfg.code=String(network[codeName].as<const char*>());
        cfg.secret=String(network[clientSecretName].as<const char*>());
    }
    return cfg;
}

void saveConfig(Configuration& cfg)
{
    DynamicJsonDocument doc(ConfigJsonBufferSize);
    JsonObject network = doc.createNestedObject(networkName);
    
    network[ssidName] = cfg.ssid.c_str();
    network[pwdName] = cfg.pwd.c_str();
    network[hostName] = cfg.host.c_str();

    network[clientIdName] = cfg.clientId.c_str();
    network[tennentName] = cfg.tennentId.c_str();
    network[codeName] = cfg.code.c_str();
    network[clientSecretName] = cfg.secret.c_str();
    
    Json::saveToFile(doc, configFileName);
}

