COM_PORT_ESPTOOL=/dev/cu.wchusbserial1420
SMING_ARCH=Esp8266
SPI_SIZE=4M
SPI_MODE=dio
COM_SPEED_ESPTOOL=115200

ARDUINO_LIBRARIES 	:= ArduinoJson6 

COMPONENT_DEPENDS += rboot
COMPONENT_DEPENDS += spiffs
COMPONENT_DEPENDS += FlashString
#COMPONENT_DEPENDS += mdns

DISABLE_SPIFFS = 0
ENABLE_SSL=Bearssl
SSL_DEBUG=0
DEBUG_VERBOSE_LEVEL=1
ENABLE_CUSTOM_HEAP=1
ENABLE_ESPCONN=0
SPIFF_SIZE      ?= 262144

#CUSTOM_TARGETS	:= files/Readme.md
# Large text file for demo purposes
#files/Readme.md: $(SMING_HOME)/../Readme.md
#	$(Q) mkdir -p $(@D)
#	$(Q) cp $< $@
	
# Emulate both serial ports
ENABLE_HOST_UARTID := 0 1
