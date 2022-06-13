#include "api.h"
#include <string.h>
#include "iota_io.h"
#include "macros.h"
#include "os.h"
#include "ui/ui.h"

#include "iota/constants.h"
#include "iota/essence.h"

#include "ui/nano/flow_user_confirm.h"

// gcc doesn't know this and ledger's SDK cannot be compiled with Werror!
//#pragma GCC diagnostic error "-Werror"
//#pragma GCC diagnostic error "-Wpedantic"
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

/// global variable storing all data needed across multiple api calls
API_CTX api;

void api_initialize()
{
    // wipe all data
    explicit_bzero(&api, sizeof(api));

    api.bip32_path[0] = 0x8000002c;
#ifdef APP_DEBUG
    api.bip32_path[1] = 0x80000001;
#else
    api.bip32_path[1] = 0x8000107a;
#endif
    api.bip32_path[BIP32_ACCOUNT_INDEX] = 0;
    api.bip32_path[BIP32_CHANGE_INDEX] = 0;
    api.bip32_path[BIP32_ADDRESS_INDEX] = 0;
}

void api_clear_data()
{
    // the only thing we shouldn't clear out is the account index
    uint32_t tmp_bip32_account = api.bip32_path[BIP32_ACCOUNT_INDEX];

#ifdef APP_DEBUG
    // and the non-interactive flag if compiled in DEBUG mode
    uint8_t tmp_non_interactive = api.non_interactive_mode;
#endif

    api_initialize();

    // set saved account index and network
    api.bip32_path[BIP32_ACCOUNT_INDEX] = tmp_bip32_account;

#ifdef APP_DEBUG
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
    resp.app_flags = !!is_locked;

#if defined(TARGET_NANOX) || defined(TARGET_NANOS2)
    resp.device = 1;
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

uint32_t api_reset()
{
    // also resets the account index
    api_initialize();

    ui_reset();

    io_send(NULL, 0, SW_OK);
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

uint32_t api_set_account(const uint8_t *data, uint32_t len)
{
    // check if a uint32_t was sent as data
    if (len != sizeof(uint32_t)) {
        THROW(SW_INCORRECT_LENGTH);
    }


    uint32_t tmp_bip32_account;
    memcpy(&tmp_bip32_account, data, sizeof(uint32_t));

    // valid bip32_account? MSB must be set
    if (!(tmp_bip32_account & 0x80000000)) {
        THROW(SW_COMMAND_INVALID_DATA);
    }

    // delete all data
    api_initialize();

    // set account index
    api.bip32_path[BIP32_ACCOUNT_INDEX] = tmp_bip32_account;

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

    // use api buffer and essence to show new address on the screen
    // buffer is free after index 32
    // writing to the buffer from external is not allowed, so data 
    // can't be changed after the flow is started
    SIG_LOCKED_SINGLE_OUTPUT *tmp =
        (SIG_LOCKED_SINGLE_OUTPUT *)&api.data.buffer[64];
    memcpy(&tmp->address_type, api.data.buffer,
              ADDRESS_WITH_TYPE_SIZE_BYTES);

    api.essence.outputs_count = 1;
    api.essence.outputs = tmp;
    api.essence.remainder_bip32.bip32_index = req.bip32_index;
    api.essence.remainder_bip32.bip32_change = req.bip32_change;
    api.essence.remainder_index = 0;

    // reset the address index to the original value
    api.bip32_path[BIP32_ADDRESS_INDEX] = req.bip32_index;
    api.bip32_path[BIP32_CHANGE_INDEX] = req.bip32_change;

    api.flow_locked = 1; // mark flow locked

    flow_start_new_address(&api, api_generate_address_accepted,
                           api_generate_address_timeout, api.bip32_path);
    return IO_ASYNCH_REPLY;
}

uint32_t api_prepare_signing(uint8_t single_sign, uint8_t has_remainder,
                             const uint8_t *data, uint32_t len)
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
        API_PREPARE_SIGNING_REQUEST req;
        memcpy(&req, data, sizeof(req));

        if (!(req.remainder_bip32_change & 0x80000000) ||
            !(req.remainder_bip32_index & 0x80000000) ||
            req.remainder_index >= OUTPUTS_MAX_COUNT) {
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

    // save into variable if single-sign mode will be used
    api.essence.single_sign_mode = !!single_sign;

    if (!essence_parse_and_validate(&api)) {
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

    // some basic checks - actually data should be 100% validated already
    if (api.data.length >= API_BUFFER_SIZE_BYTES ||
        api.essence.length >= API_BUFFER_SIZE_BYTES ||
        api.data.length < api.essence.length ||
        api.essence.inputs_count < INPUTS_MIN_COUNT ||
        api.essence.inputs_count > INPUTS_MAX_COUNT) {
        THROW(SW_UNKNOWN);
    }

    // set correct bip32 path for showing the remainder address on the UI
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

    flow_start_user_confirm(&api, &api_user_confirm_essence_accepted,
                            &api_user_confirm_essence_rejected,
                            &api_user_confirm_essence_timeout, api.bip32_path);

    return IO_ASYNCH_REPLY;
}

// prefered signing methond on the nano-x
// uses additional memory on in the buffer but only
// needs a single call to the signing method
uint32_t api_sign()
{
    if (api.data.type != USER_CONFIRMED_ESSENCE) {
        THROW(SW_COMMAND_NOT_ALLOWED);
    }

    // was not validated in single sign mode?
    if (api.essence.single_sign_mode) {
        THROW(SW_COMMAND_NOT_ALLOWED);
    }

    // some basic checks - actually data should be 100% validated already
    if (api.data.length >= API_BUFFER_SIZE_BYTES ||
        api.essence.length >= API_BUFFER_SIZE_BYTES ||
        api.data.length < api.essence.length ||
        api.essence.inputs_count < INPUTS_MIN_COUNT ||
        api.essence.inputs_count > INPUTS_MAX_COUNT) {
        THROW(SW_UNKNOWN);
    }

    uint8_t ret = essence_sign(&api);
    if (!ret) {
        THROW(SW_UNKNOWN);
    }

    api.data.type = SIGNATURES;

    io_send(NULL, 0, SW_OK);

    return 0;
}

// prefered signing methond on the nano-s
// it needs as many calls as there are inputs but needs
// no additional memory in the buffer for signatures
uint32_t api_sign_single(uint8_t p1)
{
    if (api.data.type != USER_CONFIRMED_ESSENCE) {
        THROW(SW_COMMAND_NOT_ALLOWED);
    }

    // this check actually wouldn't be needed because
    // if validation passed without single-flag, sign_single would be
    // okay but not the other way around!
    // but if validation without single-flag passes, you could use the
    // sign-api-call right away.
    //
    // so this check only is here to prevent mixing validation and signing
    // modes because it doesn't make sense and could lead to client problems.

    // was validated in single sign mode?
    if (!api.essence.single_sign_mode) {
        THROW(SW_COMMAND_NOT_ALLOWED);
    }

    // some basic checks - actually data is 100% validated already
    if (api.data.length >= API_BUFFER_SIZE_BYTES ||
        api.essence.length >= API_BUFFER_SIZE_BYTES ||
        api.data.length < api.essence.length ||
        api.essence.inputs_count < INPUTS_MIN_COUNT ||
        api.essence.inputs_count > INPUTS_MAX_COUNT) {
        THROW(SW_UNKNOWN);
    }

    if (p1 >= api.essence.inputs_count) {
        THROW(SW_INCORRECT_P1P2);
    }

    uint32_t signature_idx = p1;

    uint8_t *output = io_get_buffer();
    uint16_t signature_size_bytes = essence_sign_single(
        &api, output, sizeof(SIGNATURE_UNLOCK_BLOCK), signature_idx);

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

