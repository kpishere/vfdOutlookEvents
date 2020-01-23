#include <JsonObjectStream.h>
#include "Configuration.h"
#include "Calendar.h"
#include "webserver.h"

const uint16_t ConfigJsonBufferSize = 255;

int serverPort = 80;

DEFINE_FSTR_LOCAL(filename_login,"login.html");
DEFINE_FSTR_LOCAL(config_tennentName,"tennentid");
DEFINE_FSTR_LOCAL(config_clientIdName,"client_id");

DEFINE_FSTR_LOCAL(path_config,"/config");
DEFINE_FSTR_LOCAL(path_authorize,"/authorize");
DEFINE_FSTR_LOCAL(path_register,"/register");
DEFINE_FSTR_LOCAL(path_newToken,"/newToken");
DEFINE_FSTR_LOCAL(path_refresh,"/refresh");

DEFINE_FSTR_LOCAL(flash_fnameJQuery, "jquery-2.1.4.min.js");
DEFINE_FSTR_LOCAL(flash_fnameBootStrap, "bootstrap.min.css");

IMPORT_FSTR_LOCAL(flash_jqueryjsgz,  PROJECT_DIR  "/web/jquery-2.1.4.min.js.gz")
IMPORT_FSTR_LOCAL(flash_bootstrapcssgz,  PROJECT_DIR  "/web/bootstrap.min.css.gz")


HttpServer server;
unsigned totalActiveSockets = 0;

size_t myBodyToStringParser(HttpRequest& request, const char* at, int length)
{
    auto data = static_cast<String*>(request.args);
    
    if(length == PARSE_DATASTART) {
        delete data;
        data = new String();
        request.args = data;
        return 0;
    }
    
    if(data == nullptr) {
        debugf("Invalid request argument");
        return 0;
    }
    
    if(length == PARSE_DATAEND || length < 0) {
        request.setBody(*data);
        if(request.method == HTTP_POST) {
            if(request.getBody() == nullptr) {
                debugf("NULL bodyBuf");
                return 0;
            }

			ActiveConfig = loadConfig();
			DynamicJsonDocument root(ConfigJsonBufferSize);

			if(!Json::deserialize(root, request.getBodyStream())) {
				debug_w("Invalid JSON to un-serialize");
				return 0;
			}

			debugf("Form values: ");
			Json::serialize(root, Serial, Json::Pretty); // For debugging

			if(root.containsKey(config_tennentName)) // Settings
			{
				ActiveConfig.tennentId = root[config_tennentName].as<const char*>();
			}
			if(root.containsKey(config_clientIdName)) // Settings
			{
				ActiveConfig.clientId = root[config_clientIdName].as<const char*>();
			}
			saveConfig(ActiveConfig);
        }
        delete data;
        request.args = nullptr;
        return 0;
    }
    
    if(!data->concat(at, length)) {
        return 0;
    }
    
    return length;
}

// path = "/...."
void sendRedirect(HttpResponse& response, String path, unsigned secondsDelay = 15) {
    // Redirect client browser to download file after it is saved locally
	String respString = F("<head><meta http-equiv=\"Refresh\" content=\"");
	respString += secondsDelay;
	respString += _F("; URL=");
	respString += _F("http://");
	respString += (WifiStation.isConnected() ? WifiStation.getIP() : WifiAccessPoint.getIP()).toString();
	respString += ':';
	respString += serverPort;
	respString += path;
	respString += _F("\"/></head>");
    
    response.setCache(0,true);
    response.sendString(respString);
}

void onFile(HttpRequest& request, HttpResponse& response)
{
    String file = request.uri.getRelativePath();
    
    if(file[0] == '.') { // leading period on files -- don't send
        response.code = HTTP_STATUS_FORBIDDEN;
        return;
    }

    response.setCache(86400, true); // It's important to use cache for better performance.
    if(file == "") {
        response.sendFile(_F("index.html"));
    } else if(file == flash_fnameJQuery) {
        response.headers[HTTP_HEADER_CONTENT_ENCODING] = _F("gzip");
        auto stream = new FlashMemoryStream(flash_jqueryjsgz);
        response.sendDataStream(stream, MIME_JS);
    } else if(file == flash_fnameBootStrap) {
        response.headers[HTTP_HEADER_CONTENT_ENCODING] = _F("gzip");
        auto stream = new FlashMemoryStream(flash_bootstrapcssgz);
        response.sendDataStream(stream, MIME_CSS);
    } else if(file == filename_login) { // This is for re-attempts at login
        response.headers[HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN] = "*";
        response.sendFile(file);
    } else {
        response.sendFile(file);
    }
}

void onConfig(HttpRequest& request, HttpResponse& response)
{
    if(request.method == HTTP_GET) {
        JsonObjectStream* stream = new JsonObjectStream();
        JsonObject json = stream->getRoot();
        
        json[config_tennentName] = ActiveConfig.tennentId.c_str();
        json[config_clientIdName] = ActiveConfig.clientId.c_str();
        
        //Json::serialize(json, Serial, Json::Pretty); // For debugging

        response.setCache(0,true);
        response.sendDataStream(stream, MIME_JSON);
    }
}

void onAuthorize(HttpRequest& request, HttpResponse& response)
{
    Calendar::getAuthorization(filename_login);
    
    sendRedirect(response, String('/') + filename_login);
}

/*
 * If one could run an HTTPS service, this could be called and code value set here.
 * Alternately, use serial interface to set 'code' value.
 */
void onRegister(HttpRequest& request, HttpResponse& response) {
    Calendar::onCodeSave( request.uri.Query );
    sendRedirect(response, path_newToken);
}

void onGetToken(HttpRequest& request, HttpResponse& response)
{
    Calendar::getToken();
    sendRedirect(response, "/");
}

void onCalRefresh(HttpRequest& request, HttpResponse& response)
{
    Calendar::getCalendar();
    sendRedirect(response, "/", 1);
}

//mDNS using ESP8266 SDK functions
void startmDNS()
{
#ifdef ENABLE_ESPCONN
    struct mdns_info* info = (struct mdns_info*)malloc(sizeof(struct mdns_info));
    info->host_name = (char *)ActiveConfig.host.c_str();
    info->ipAddr = WifiStation.getIP();
    info->server_name = info->host_name;
    info->server_port = serverPort;
    info->txt_data[0] = (char *)"";
    //espconn_mdns_init(info);
#endif
}

void startWebServer()
{    
    System.setCpuFrequency(eCF_160MHz);
    Serial.print(F("New CPU frequency is: "));
    Serial.println(System.getCpuFrequency());
    /*
     #include "ssl/server_cert.h"
     #include "ssl/server_private_key.h"
    SSLKeyCertPair clientCertKey;
    
    clientCertKey.certificate = new uint8_t[default_certificate_len];
    memcpy(clientCertKey.certificate, default_certificate,
           default_certificate_len);
    clientCertKey.certificateLength = default_certificate_len;
    clientCertKey.key = new uint8_t[default_private_key_len];
    memcpy(clientCertKey.key, default_private_key, default_private_key_len);
    clientCertKey.keyLength = default_private_key_len;
    clientCertKey.keyPassword = NULL;
    
    server.setServerKeyCert(clientCertKey);
     server.listen(serverPort,true);
     */
    server.listen(serverPort,false);
    server.paths.set(path_config, onConfig);
    server.paths.set(path_authorize, onAuthorize);
    server.paths.set(path_register,onRegister);
    server.paths.set(path_newToken,onGetToken);
    server.paths.set(path_refresh,onCalRefresh);
    
    server.paths.setDefault(onFile);
    server.setBodyParser(MIME_JSON,myBodyToStringParser);

    Serial.println(F("\r\n=== WEB SERVER STARTED ==="));
    Serial.println(WifiStation.getIP());
    Serial.println(F("==============================\r\n"));
    
    // Schedule poll event - only called after a token refresh
    Calendar::getCalendarIn(10 * 1000);
}
