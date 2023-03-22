# ****************************************************************************
#   Ledger App
#   (c) 2017 Ledger
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
# ****************************************************************************

ifeq ($(BOLOS_SDK),)
$(error Environment variable BOLOS_SDK is not set)
endif

include $(BOLOS_SDK)/Makefile.defines

APPVERSION_M = 0
APPVERSION_N = 8
APPVERSION_P = 5
APPVERSION   = "$(APPVERSION_M).$(APPVERSION_N).$(APPVERSION_P)"

APP_LOAD_PARAMS = --path "44'/1'" --curve ed25519 --appFlags 0x240 $(COMMON_LOAD_PARAMS)


ifeq ($(CHAIN),)
CHAIN=iota
endif

# Check if chain is available
ifeq ($(shell test -s ./makefile_conf/chain/$(CHAIN).mk && echo -n yes), yes)
include ./makefile_conf/chain/$(CHAIN).mk
else
$(error Unsupported CHAIN - use $(SUPPORTED_CHAINS))
endif

all: default

DEFINES += $(DEFINES_LIB)
DEFINES += APPNAME=\"$(APPNAME)\"
DEFINES += APPVERSION=\"$(APPVERSION)\"
DEFINES += APPVERSION_MAJOR=$(APPVERSION_M) APPVERSION_MINOR=$(APPVERSION_N) APPVERSION_PATCH=$(APPVERSION_P)
DEFINES += OS_IO_SEPROXYHAL
DEFINES += HAVE_BAGL HAVE_UX_FLOW HAVE_SPRINTF HAVE_SNPRINTF_FORMAT_U
DEFINES += HAVE_IO_USB HAVE_L4_USBLIB IO_USB_MAX_ENDPOINTS=6 IO_HID_EP_LENGTH=64 HAVE_USB_APDU
DEFINES += USB_SEGMENT_SIZE=64
DEFINES += BLE_SEGMENT_SIZE=32
DEFINES += HAVE_WEBUSB WEBUSB_URL_SIZE_B=0 WEBUSB_URL=""
DEFINES += UNUSED\(x\)=\(void\)x

ifeq ($(TARGET_NAME),TARGET_NANOX)
    DEFINES += HAVE_BLE BLE_COMMAND_TIMEOUT_MS=2000 HAVE_BLE_APDU
endif

ifeq ($(TARGET_NAME),TARGET_NANOS)
    DEFINES += IO_SEPROXYHAL_BUFFER_SIZE_B=128
else
	# nanox, nanosplus
    DEFINES += IO_SEPROXYHAL_BUFFER_SIZE_B=300
    DEFINES += HAVE_GLO096
    DEFINES += BAGL_WIDTH=128 BAGL_HEIGHT=64
    DEFINES += HAVE_BAGL_ELLIPSIS
    DEFINES += HAVE_BAGL_FONT_OPEN_SANS_REGULAR_11PX
    DEFINES += HAVE_BAGL_FONT_OPEN_SANS_EXTRABOLD_11PX
    DEFINES += HAVE_BAGL_FONT_OPEN_SANS_LIGHT_16PX
endif

# default disable DEBUG build
DEBUG = 0

# if speculos simulator is selected enable debuging features
ifeq ($(SPECULOS), 1)
DEFINES += SPECULOS
DEBUG = 1
endif

ifneq ($(DEBUG),0)
    APP_LOAD_PARAMS += --path "44'/01'"
    DEFINES += HAVE_BOLOS_APP_STACK_CANARY
    DEFINES += APP_DEBUG

    # we don't need printf
    DEFINES += HAVE_PRINTF PRINTF=
else
    # Release flags
    DEFINES += PRINTF\(...\)=
endif

ifneq ($(BOLOS_ENV),)
$(info BOLOS_ENV=$(BOLOS_ENV))
CLANGPATH := $(BOLOS_ENV)/clang-arm-fropi/bin/
GCCPATH   := $(BOLOS_ENV)/gcc-arm-none-eabi-5_3-2016q1/bin/
else
$(info BOLOS_ENV is not set: falling back to CLANGPATH and GCCPATH)
endif
ifeq ($(CLANGPATH),)
$(info CLANGPATH is not set: clang will be used from PATH)
endif
ifeq ($(GCCPATH),)
$(info GCCPATH is not set: arm-none-eabi-* will be used from PATH)
endif

CC      := $(CLANGPATH)clang

AS := $(GCCPATH)arm-none-eabi-gcc
AFLAGS +=

LD := $(GCCPATH)arm-none-eabi-gcc

ifeq ($(DEBUG),1)
LDFLAGS += -O0 -g3
else
LDFLAGS += -O2
endif

LDLIBS += -lm -lgcc -lc

# import rules to compile glyphs(/pone)
include $(BOLOS_SDK)/Makefile.glyphs

APP_SOURCE_PATH += src
SDK_SOURCE_PATH += lib_stusb lib_stusb_impl lib_ux

ifeq ($(TARGET_NAME),TARGET_NANOX)
    SDK_SOURCE_PATH += lib_blewbxx lib_blewbxx_impl
endif

load: all
	python3 -m ledgerblue.loadApp $(APP_LOAD_PARAMS)

load-offline: all
	python3 -m ledgerblue.loadApp $(APP_LOAD_PARAMS) --offline

delete:
	python3 -m ledgerblue.deleteApp $(COMMON_DELETE_PARAMS)

include $(BOLOS_SDK)/Makefile.rules

dep/%.d: %.c Makefile

listvariants:
	@echo VARIANTS CHAIN iota shimmer
