APPNAME      = "Shimmer"
APPVERSION_M = 0
APPVERSION_N = 8
APPVERSION_P = 2
APPVERSION   = "$(APPVERSION_M).$(APPVERSION_N).$(APPVERSION_P)"

APP_LOAD_PARAMS = --path "44'/1'" --curve ed25519 --appFlags 0x240 $(COMMON_LOAD_PARAMS)

DEFINES += APP_SHIMMER
# IOTA BIP-path for claiming Shimmer from IOTA addresses
APP_LOAD_PARAMS += --path "44'/4218'"
# Shimmer BIP-path
APP_LOAD_PARAMS += --path "44'/4219'"

ifeq ($(TARGET_NAME),TARGET_NANOS)
    ICONNAME=icons/nanos_app_shimmer.gif
else
    ICONNAME=icons/nanox_app_shimmer.gif
endif

