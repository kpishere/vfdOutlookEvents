#include <JsonObjectStream.h>
#include "Configuration.h"

const uint16_t ConfigJsonBufferSize = 2048;


const char endOfLine = '\n';
DEFINE_FSTR_LOCAL(configFileName,".config.conf");  // leading period for files un-downloadable by webserver
DEFINE_FSTR_LOCAL(networkName,"network");
DEFINE_FSTR_LOCAL(ssidName,"StaSSID");
DEFINE_FSTR_LOCAL(pwdName,"StaPassword");
DEFINE_FSTR_LOCAL(hostName,"HostName");

DEFINE_FSTR_LOCAL(tennentName,"tennentid");
DEFINE_FSTR_LOCAL(clientIdName,"clientid");
DEFINE_FSTR_LOCAL(codeName,"code");
DEFINE_FSTR_LOCAL(clientSecretName,"secret");

Configuration ActiveConfig;

Configuration loadConfig()
{
    DynamicJsonDocument doc(ConfigJsonBufferSize);
    Configuration cfg;
    
    if(Json::loadFromFile(doc, configFileName)) {
        //debug_i("Load values: ");
        //Json::serialize(doc, Serial, Json::Pretty); // For debugging

        JsonObject network = doc[networkName];
        cfg.ssid = network[ssidName].as<const char*>();
        cfg.pwd = network[pwdName].as<const char*>();
        cfg.host = network[hostName].as<const char*>();
        
        cfg.tennentId=network[tennentName].as<const char*>();
        cfg.clientId=network[clientIdName].as<const char*>();
        cfg.code=network[codeName].as<const char*>();
        cfg.secret=network[clientSecretName].as<const char*>();
    }
    return cfg;
}

void saveConfig(Configuration& cfg)
{
    DynamicJsonDocument doc(ConfigJsonBufferSize);
    JsonObject network = doc.createNestedObject(networkName);
    
    network[ssidName] = cfg.ssid;
    network[pwdName] = cfg.pwd;
    network[hostName] = cfg.host;

    network[clientIdName] = cfg.clientId;
    network[tennentName] = cfg.tennentId;
    network[codeName] = cfg.code;
    network[clientSecretName] = cfg.secret;
    
    Json::saveToFile(doc, configFileName);
}

