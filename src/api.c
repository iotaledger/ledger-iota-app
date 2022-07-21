#include "os.h"

#include <string.h>

#include "iota_io.h"
#include "api.h"

#include "macros.h"

#include "ui/ui.h"

#include "nv_mem.h"

#include "abstraction.h"

#include "iota/constants.h"
#include "iota/blindsigning_stardust.h"
#include "iota/signing.h"

#include "ui/nano/flow_user_confirm_transaction.h"
#include "ui/nano/flow_user_confirm_new_address.h"
#include "ui/nano/flow_user_confirm_blindsigning.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

/// global variable storing all data needed across multiple api calls
API_CTX api;

void api_initialize(APP_MODE_TYPE app_mode, uint32_t account_index)
{
    // wipe all data
    explicit_bzero(&api, sizeof(api));

    api.bip32_path[0] = 0x8000002c;

    // app-modes
    // IOTA App
    // 0x00: (107a) IOTA + Chrysalis (default, backwards compatible)
    // 0x80:    (1) IOTA + Chrysalis Testnet
    // 0x01: (107a) IOTA + Stardust
    // 0x81:    (1) IOTA + Stardust Testnet

    // Shimmer App
    // 0x02: (107a) Shimmer Claiming (from IOTA)
    // 0x82:    (1) Shimmer Claiming (from IOTA) (Testnet)
    // 0x03: (107b) Shimmer (default)
    // 0x83:    (1) Shimmer Testnet

    switch (app_mode & 0x7f) {
#if defined(APP_IOTA)
    case APP_MODE_IOTA_CHRYSALIS:
        // iota
        api.bip32_path[BIP32_COIN_INDEX] = BIP32_COIN_IOTA;
        api.protocol = PROTOCOL_CHRYSALIS;
        api.coin = COIN_IOTA;
        break;
    case APP_MODE_IOTA_STARDUST:
        // iota
        api.bip32_path[BIP32_COIN_INDEX] = BIP32_COIN_IOTA;
        api.protocol = PROTOCOL_STARDUST;
        api.coin = COIN_IOTA;
        break;
#elif defined(APP_SHIMMER)
    case APP_MODE_SHIMMER_CLAIMING:
        // iota
        api.bip32_path[BIP32_COIN_INDEX] = BIP32_COIN_IOTA;
        api.protocol = PROTOCOL_STARDUST;
        api.coin = COIN_SHIMMER;
        break;
    case APP_MODE_SHIMMER:
        // shimmer
        api.bip32_path[BIP32_COIN_INDEX] = BIP32_COIN_SHIMMER;
        api.protocol = PROTOCOL_STARDUST;
        api.coin = COIN_SHIMMER;
        break;
#else
#error unknown app
#endif
    default:
        THROW(SW_ACCOUNT_NOT_VALID);
    }

    // set bip paths for testnet
    if (app_mode & 0x80) {
        api.bip32_path[BIP32_COIN_INDEX] = BIP32_COIN_TESTNET;
    }

    // set account indices
    // value of 0 is allowed because it tells us that no account was set
    // through the api and it is checked in every api function that uses
    // account indices.
    api.bip32_path[BIP32_ACCOUNT_INDEX] = account_index;

    api.app_mode = app_mode;
}

uint32_t api_reset()
{
    // also resets the account index
    api_initialize(APP_MODE_INIT, 0);

    ui_reset();

    io_send(NULL, 0, SW_OK);
    return 0;
}


void api_clear_data()
{
#ifdef APP_DEBUG
    // save the non-interactive flag if compiled in DEBUG mode
    uint8_t tmp_non_interactive = api.non_interactive_mode;
#endif

    // initialize with previously set app-mode and account index
    api_initialize(api.app_mode, api.bip32_path[BIP32_ACCOUNT_INDEX]);

#ifdef APP_DEBUG
    // restore non-interactive flag
    api.non_interactive_mode = tmp_non_interactive;
#endif
}

uint32_t api_write_data_block(uint8_t block_number, const uint8_t *input_data,
                              uint32_t len)
{
    // only allow write on empty buffer
    if (api.data.type != EMPTY) {
        THROW(SW_COMMAND_NOT_ALLOWED);
    }
    // only accept payload with exactly data_block_size length
    if (len != DATA_BLOCK_SIZE) {
        THROW(SW_INCORRECT_LENGTH);
    }

    // check if chunk-number [0..MAX_NUM_DATA_BLOCKS)
    if (block_number >= DATA_BLOCK_COUNT) {
        THROW(SW_INCORRECT_P1P2);
    }

    memcpy(&api.data.buffer[block_number * DATA_BLOCK_SIZE], input_data,
           DATA_BLOCK_SIZE);

    io_send(NULL, 0, SW_OK);

    return 0;
}

uint32_t api_read_data_block(uint8_t block_number)
{
    if (api.data.type != GENERATED_ADDRESSES && api.data.type != SIGNATURES) {
        THROW(SW_COMMAND_NOT_ALLOWED);
    }

    // check if chunk-number [0..MAX_NUM_DATA_BLOCKS)
    if (block_number >= DATA_BLOCK_COUNT) {
        THROW(SW_INCORRECT_P1P2);
    }

    io_send(&api.data.buffer[block_number * DATA_BLOCK_SIZE], DATA_BLOCK_SIZE,
            SW_OK);

    return 0;
}

uint32_t api_get_data_buffer_state()
{
    API_GET_DATA_BUFFER_STATE_RESPONSE resp;
    resp.data_length = api.data.length;
    resp.data_type = (uint8_t)api.data.type;
    resp.data_block_count = DATA_BLOCK_COUNT;
    resp.data_block_size = DATA_BLOCK_SIZE;

    io_send(&resp, sizeof(resp), SW_OK);
    return 0;
}

uint32_t api_clear_data_buffer()
{
    // wipe all including other api-flags
    api_clear_data();

    io_send(NULL, 0, SW_OK);
    return 0;
}

// get application configuration (flags and version)
uint32_t api_get_app_config(uint8_t is_locked)
{
    API_GET_APP_CONFIG_RESPONSE resp;
    resp.app_version_major = APPVERSION_MAJOR;
    resp.app_version_minor = APPVERSION_MINOR;
    resp.app_version_patch = APPVERSION_PATCH;

    // bit 0: locked
    // bit 1: blindsigning enabled
    // bit 2: IOTA / Shimmer app
    resp.app_flags = !!is_locked;
    resp.app_flags |= !!nv_get_blindsigning() << 1;

#if defined(APP_IOTA)
    // actually not needed because bit is cleared anyways
    resp.app_flags &= ~(1 << 2);
#elif defined(APP_SHIMMER)
    resp.app_flags |= (1 << 2);
#else
#error unknown app
#endif

#if defined(TARGET_NANOX)
    resp.device = 1;
#elif defined(TARGET_NANOS2)
    resp.device = 2;
#else
    resp.device = 0;
#endif

#ifdef APP_DEBUG
    resp.debug = 1;
#else
    resp.debug = 0;
#endif

    io_send(&resp, sizeof(resp), SW_OK);
    return 0;
}


uint32_t api_show_flow(uint8_t flow)
{
    if (!ui_show(flow)) {
        THROW(SW_INCORRECT_P1P2);
    }

    io_send(NULL, 0, SW_OK);
    return 0;
}

uint32_t api_set_account(uint8_t app_mode, const uint8_t *data, uint32_t len)
{
    // check if a uint32_t was sent as data
    if (len != sizeof(uint32_t)) {
        THROW(SW_INCORRECT_LENGTH);
    }

    if ((app_mode & 0x7f) > APP_MODE_SHIMMER) {
        THROW(SW_INCORRECT_P1P2);
    }

    uint32_t tmp_bip32_account;
    memcpy(&tmp_bip32_account, data, sizeof(uint32_t));

    // valid bip32_account? MSB must be set
    if (!(tmp_bip32_account & 0x80000000)) {
        THROW(SW_COMMAND_INVALID_DATA);
    }

    // delete all data, set app_mode and account index
    api_initialize(app_mode, tmp_bip32_account);

    io_send(NULL, 0, SW_OK);

    return 0;
}

// callback for acknowledging a new (remainder) address
static void api_generate_address_accepted()
{
    // in interactive flows set data_type here, so data is not readable
    // before acknowleding
    api.data.type = GENERATED_ADDRESSES;
    io_send(NULL, 0, SW_OK);
}

// callback for timeout
static void api_generate_address_timeout()
{
    api_clear_data();
    io_send(NULL, 0, SW_COMMAND_TIMEOUT);
}

uint32_t api_generate_address(uint8_t show_on_screen, const uint8_t *data,
                              uint32_t len)
{
    // don't allow command if an interactive flow already is running
    if (api.flow_locked) {
        THROW(SW_COMMAND_NOT_ALLOWED);
    }
    // was account selected?
    if (!(api.bip32_path[BIP32_ACCOUNT_INDEX] & 0x80000000)) {
        THROW(SW_ACCOUNT_NOT_VALID);
    }

    // if buffer contains data, throw exception
    if (api.data.type != EMPTY) {
        THROW(SW_COMMAND_NOT_ALLOWED);
    }

    // disable external read and write access before doing anything else
    api.data.type = LOCKED;

    if (len != sizeof(API_GENERATE_ADDRESS_REQUEST)) {
        THROW(SW_INCORRECT_LENGTH);
    }

    API_GENERATE_ADDRESS_REQUEST req;
    memcpy(&req, data, sizeof(req));

    // check if too many addresses to generate
    if (req.count > API_GENERATE_ADDRESSES_MAX_COUNT) {
        THROW(SW_COMMAND_INVALID_DATA);
    }

    // if new address will be shown in a flow, only allow a single address
    // this mode is used for generating new remainder addresses and show
    // it to the user to review
    if (show_on_screen && req.count != 1) {
        THROW(SW_COMMAND_INVALID_DATA);
    }

    // check if MSBs set
    if (!(req.bip32_index & 0x80000000) || !(req.bip32_change & 0x80000000)) {
        THROW(SW_COMMAND_INVALID_DATA);
    }

    // bip32 change can be 0x80000000 or 0x80000001
    if (req.bip32_change & 0x7ffffffe) {
        THROW(SW_COMMAND_INVALID_DATA);
    }

    // check if there would be an overflow when generating addresses
    if (!((req.bip32_index + req.count) & 0x80000000)) {
        THROW(SW_COMMAND_INVALID_DATA);
    }

    api.bip32_path[BIP32_ADDRESS_INDEX] = req.bip32_index;
    api.bip32_path[BIP32_CHANGE_INDEX] = req.bip32_change;

    // wipe all data before buffer is used again
    memset(api.data.buffer, 0, API_BUFFER_SIZE_BYTES);
    for (uint32_t i = 0; i < req.count; i++) {
        // with address_type
        uint8_t ret = address_generate(
            api.bip32_path, BIP32_PATH_LEN,
            &api.data.buffer[i * ADDRESS_WITH_TYPE_SIZE_BYTES]);

        if (!ret) {
            THROW(SW_UNKNOWN);
        }
        // generate next address
        api.bip32_path[BIP32_ADDRESS_INDEX]++;
    }

    api.data.length = req.count * ADDRESS_WITH_TYPE_SIZE_BYTES;

    if (!show_on_screen ||
#ifdef APP_DEBUG
        api.non_interactive_mode
#else
        false
#endif
    ) {
        api.data.type = GENERATED_ADDRESSES;
        io_send(NULL, 0, SW_OK);
        return 0;
    }

    api.essence.outputs_count = 1;
    api.essence.remainder_bip32.bip32_index = req.bip32_index;
    api.essence.remainder_bip32.bip32_change = req.bip32_change;
    api.essence.remainder_index = 0;

    // reset the address index to the original value
    api.bip32_path[BIP32_ADDRESS_INDEX] = req.bip32_index;
    api.bip32_path[BIP32_CHANGE_INDEX] = req.bip32_change;

    api.flow_locked = 1; // mark flow locked

    flow_start_new_address(&api, api_generate_address_accepted,
                           api_generate_address_timeout);
    return IO_ASYNCH_REPLY;
}

uint32_t api_prepare_signing(uint8_t has_remainder, const uint8_t *data,
                             uint32_t len)
{
    // when calling validation the buffer still is marked as empty
    if (api.data.type != EMPTY) {
        THROW(SW_COMMAND_NOT_ALLOWED);
    }

    // disable external read and write access before doing anything else
    api.data.type = LOCKED;

    // was account selected?
    if (!(api.bip32_path[BIP32_ACCOUNT_INDEX] & 0x80000000)) {
        THROW(SW_ACCOUNT_NOT_VALID);
    }

    if (len != sizeof(API_PREPARE_SIGNING_REQUEST)) {
        THROW(SW_INCORRECT_LENGTH);
    }

    // if essence has an remainder, store the information about
    if (!!has_remainder) {
        // no remainder for claiming shimmer allowed
        if (api.app_mode == APP_MODE_SHIMMER_CLAIMING) {
            THROW(SW_COMMAND_INVALID_DATA);
        }

        API_PREPARE_SIGNING_REQUEST req;
        memcpy(&req, data, sizeof(req));

        if (!(req.remainder_bip32_change & 0x80000000) ||
            !(req.remainder_bip32_index & 0x80000000)) {
            THROW(SW_COMMAND_INVALID_DATA);
        }

        if ((api.protocol == PROTOCOL_CHRYSALIS &&
             req.remainder_index >= OUTPUTS_MAX_COUNT_CHRYSALIS) ||
            (api.protocol == PROTOCOL_STARDUST &&
             req.remainder_index >= OUTPUTS_MAX_COUNT_STARDUST)) {
            THROW(SW_COMMAND_INVALID_DATA);
        }

        api.essence.has_remainder = 1;
        api.essence.remainder_index = req.remainder_index;
        api.essence.remainder_bip32.bip32_index = req.remainder_bip32_index;
        api.essence.remainder_bip32.bip32_change = req.remainder_bip32_change;
    }
    else {
        api.essence.has_remainder = 0;
    }

    // no blindsigning
    api.essence.blindsigning = 0;

    if (!essence_parse_and_validate(&api)) {
        THROW(SW_COMMAND_INVALID_DATA);
    }

    api.data.type = VALIDATED_ESSENCE;

    io_send(NULL, 0, SW_OK);
    return 0;
}

uint32_t api_prepare_blindsigning()
{
    // when calling validation the buffer still is marked as empty
    if (api.data.type != EMPTY) {
        THROW(SW_COMMAND_NOT_ALLOWED);
    }

    // blindsigning only allowed with stardust protocol but not SMR claiming
    if (api.protocol != PROTOCOL_STARDUST ||
        api.app_mode == APP_MODE_SHIMMER_CLAIMING) {
        THROW(SW_COMMAND_NOT_ALLOWED);
    }

    // disable external read and write access before doing anything else
    api.data.type = LOCKED;

    // was account selected?
    if (!(api.bip32_path[BIP32_ACCOUNT_INDEX] & 0x80000000)) {
        THROW(SW_ACCOUNT_NOT_VALID);
    }

    // set flag for blindsigning
    api.essence.blindsigning = 1;

    // we allow to prepare without blindsigning enabled but the user will only
    // get an error message that blindsigning is not enabled on the Nano when
    // trying to sign what is the most consistent behaviour because the outcome
    // is the same as rejecting the signing (the flow only has a reject button
    // in this case and accepting is not possible) and we don't have to cope
    // with additional errors.
    if (!parse_and_validate_blindsigning(&api)) {
        THROW(SW_COMMAND_INVALID_DATA);
    }

    api.data.type = VALIDATED_ESSENCE;

    io_send(NULL, 0, SW_OK);
    return 0;
}

// callback for accept transaction
static void api_user_confirm_essence_accepted()
{
    api.data.type = USER_CONFIRMED_ESSENCE;
    io_send(NULL, 0, SW_OK);
}

// callback for rejected transaction
static void api_user_confirm_essence_rejected()
{
    api_clear_data();
    io_send(NULL, 0, SW_DENIED_BY_USER);
}

// callback for timeout
static void api_user_confirm_essence_timeout()
{
    api_clear_data();
    io_send(NULL, 0, SW_COMMAND_TIMEOUT);
}

uint32_t api_user_confirm_essence()
{
    // don't allow command if an interactive flow already is running
    if (api.flow_locked) {
        THROW(SW_COMMAND_NOT_ALLOWED);
    }

    if (api.data.type != VALIDATED_ESSENCE) {
        THROW(SW_COMMAND_NOT_ALLOWED);
    }

    // was account selected?
    if (!(api.bip32_path[BIP32_ACCOUNT_INDEX] & 0x80000000)) {
        THROW(SW_ACCOUNT_NOT_VALID);
    }

    if (!api.essence.blindsigning) {
        // normal flow without blindsigning

        // some basic checks - actually data should be 100% validated
        // already
        if (api.data.length >= API_BUFFER_SIZE_BYTES ||
            api.essence.length >= API_BUFFER_SIZE_BYTES ||
            api.data.length < api.essence.length ||
            api.essence.inputs_count < INPUTS_MIN_COUNT ||
            (api.protocol == PROTOCOL_CHRYSALIS &&
             api.essence.inputs_count > INPUTS_MAX_COUNT_CHRYSALIS) ||
            (api.protocol == PROTOCOL_STARDUST &&
             api.essence.inputs_count > INPUTS_MAX_COUNT_STARDUST)) {
            THROW(SW_UNKNOWN);
        }

        // set correct bip32 path for showing the remainder address on the
        // UI
        api.bip32_path[BIP32_ADDRESS_INDEX] =
            api.essence.remainder_bip32.bip32_index;
        api.bip32_path[BIP32_CHANGE_INDEX] =
            api.essence.remainder_bip32.bip32_change;

#ifdef APP_DEBUG
        if (api.non_interactive_mode) {
            api.data.type = USER_CONFIRMED_ESSENCE;

            io_send(NULL, 0, SW_OK);
            return 0;
        }
#endif
        api.flow_locked = 1; // mark flow locked

        flow_start_user_confirm_transaction(&api,
                                            &api_user_confirm_essence_accepted,
                                            &api_user_confirm_essence_rejected,
                                            &api_user_confirm_essence_timeout);
    }
    else {
// start flow for blindsigning
#ifdef APP_DEBUG
        if (api.non_interactive_mode) {
            api.data.type = USER_CONFIRMED_ESSENCE;

            io_send(NULL, 0, SW_OK);
            return 0;
        }
#endif

        api.flow_locked = 1; // mark flow locked

        // we can use the same callbacks
        flow_start_blindsigning(&api, &api_user_confirm_essence_accepted,
                                &api_user_confirm_essence_rejected,
                                &api_user_confirm_essence_timeout);
    }
    return IO_ASYNCH_REPLY;
}

// prefered signing methond on the nano-s
// it needs as many calls as there are inputs but needs
// no additional memory in the buffer for signatures
uint32_t api_sign(uint8_t p1)
{
    if (api.data.type != USER_CONFIRMED_ESSENCE) {
        THROW(SW_COMMAND_NOT_ALLOWED);
    }

    // some basic checks - actually data is 100% validated already
    if (api.data.length >= API_BUFFER_SIZE_BYTES ||
        api.essence.length >= API_BUFFER_SIZE_BYTES ||
        api.data.length < api.essence.length ||
        api.essence.inputs_count < INPUTS_MIN_COUNT ||
        (api.protocol == PROTOCOL_CHRYSALIS &&
         api.essence.inputs_count > INPUTS_MAX_COUNT_CHRYSALIS) ||
        (api.protocol == PROTOCOL_STARDUST &&
         api.essence.inputs_count > INPUTS_MAX_COUNT_STARDUST)) {
        THROW(SW_UNKNOWN);
    }

    if (p1 >= api.essence.inputs_count) {
        THROW(SW_INCORRECT_P1P2);
    }

    // check the buffer size
    if (IO_APDU_BUFFER_SIZE < sizeof(SIGNATURE_UNLOCK_BLOCK)) {
        THROW(SW_UNKNOWN);
    }

    uint32_t signature_idx = p1;

    uint8_t *output = io_get_buffer();
    uint16_t signature_size_bytes = sign(&api, output, signature_idx);

    if (!signature_size_bytes) {
        THROW(SW_UNKNOWN);
    }

    io_send(output, signature_size_bytes, SW_OK);

    return 0;
}

#ifdef APP_DEBUG
// function to verify the stack never grows into the bss segment
// by dumping the whole memory and having a look at the stack section
// in the dump. Also useful for figuring out how much empty memory after
// the bss section is left. RAM is initialized by BOLOS with 0xa5. So
// it's easy to see what memory never was written to
// DON'T enable it in production
uint32_t api_dump_memory(uint8_t pagenr)
{
#if defined(TARGET_NANOX) || defined(TARGET_NANOS2)
    // same size and location on Nano X and Nano S+
    if (pagenr >= 30 * 1024 / 128) {
        THROW(SW_INCORRECT_P1P2);
    }
    uint32_t *p = (uint32_t *)(0xda7a0000 + pagenr * 128);
#else
    // works for firmware 2.0.0
    if (pagenr >= (4096 + 512) / 128) {
        THROW(SW_INCORRECT_P1P2);
    }
    uint32_t *p = (uint32_t *)(0x20000200 + pagenr * 128);
#endif
    io_send(p, 128, SW_OK);
    return 0;
}

// set non-interactive-mode for automatic testing via speculos
uint32_t api_set_non_interactive_mode(uint8_t mode)
{
    if (mode > 1) {
        THROW(SW_INCORRECT_P1P2);
    }
    api.non_interactive_mode = mode;

    io_send(NULL, 0, SW_OK);
    return 0;
}

#endif
