APPNAME      = "IOTA"
APPVERSION_M = 0
APPVERSION_N = 8
APPVERSION_P = 0
APPVERSION   = "$(APPVERSION_M).$(APPVERSION_N).$(APPVERSION_P)"

APP_LOAD_PARAMS = --path "44'/1'" --curve ed25519 --appFlags 0x240 $(COMMON_LOAD_PARAMS)

DEFINES += APP_IOTA
# IOTA BIP-path
APP_LOAD_PARAMS += --path "44'/4218'"

ifeq ($(TARGET_NAME),TARGET_NANOS)
    ICONNAME=icons/nanos_app_iota.gif
else
    ICONNAME=icons/nanox_app_iota.gif
endif

