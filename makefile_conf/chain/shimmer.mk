APPNAME = "Shimmer"

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

