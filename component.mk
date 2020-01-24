COM_PORT_ESPTOOL=/dev/cu.wchusbserial1420
SPI_SIZE=4m
SPI_MODE=dio

ARDUINO_LIBRARIES 	:= ArduinoJson6 

#COMPONENT_INCDIRS := $(COMPONENT_INCDIRS) /Volumes/case-sensitive/bearssl-esp8266/inc
#COMPONENT_LIBDIR  := /Volumes/case-sensitive/bearssl-esp8266/esp8266
#COMPONENT_LIBNAME := bearssl

DISABLE_SPIFFS = 0
ENABLE_SSL=Bearssl
SSL_DEBUG=0
DEBUG_VERBOSE_LEVEL=1
ENABLE_CUSTOM_HEAP=1
ENABLE_MDNS=1
SPIFF_SIZE      ?= 262144

#CUSTOM_TARGETS	:= files/Readme.md
# Large text file for demo purposes
#files/Readme.md: $(SMING_HOME)/../Readme.md
#	$(Q) mkdir -p $(@D)
#	$(Q) cp $< $@
	
# Emulate both serial ports
ENABLE_HOST_UARTID := 0 1
