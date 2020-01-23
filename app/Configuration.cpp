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

