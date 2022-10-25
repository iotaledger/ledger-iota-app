APPNAME = "IOTA"

DEFINES += APP_IOTA

# IOTA BIP-path
APP_LOAD_PARAMS += --path "44'/4218'"

ifeq ($(TARGET_NAME),TARGET_NANOS)
    ICONNAME=icons/nanos_app_iota.gif
else
    ICONNAME=icons/nanox_app_iota.gif
endif

