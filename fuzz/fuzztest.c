#include <stdint.h>
#include <string.h>

#include "api.h"
#include "essence.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
	if (Size >= API_BUFFER_SIZE_BYTES) {
        API_CTX api = {0};
        memcpy(api.data.buffer, Data, API_BUFFER_SIZE_BYTES);
        // api.essence.has_remainder = 1;
        essence_parse_and_validate_chrysalis(&api);
    }
  return 0;
}