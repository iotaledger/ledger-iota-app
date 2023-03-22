#include <stdint.h>
#include "os.h"
#include "os_pic.h"
#include "os_nvm.h"
#include "nv_mem.h"

#define N_nv_mem (*(volatile nv_mem_t *)PIC(&N_nv_mem_real))

typedef struct nv_mem_t {
    uint8_t blindsigning;
    uint8_t initialized;
} nv_mem_t;

const nv_mem_t N_nv_mem_real;

void nv_init()
{
    // initialize non volatile memory
    if (N_nv_mem.initialized != 0xa5) {
        nv_mem_t nv;
        nv.blindsigning = 0x00; // blind signing not allowed per default
        nv.initialized = 0xa5;
        nvm_write((void *)&N_nv_mem, (void *)&nv, sizeof(nv_mem_t));
    }
}

uint8_t nv_get_blindsigning()
{
    return N_nv_mem.blindsigning;
}

void nv_toggle_blindsigning()
{
    uint8_t value = 1 - N_nv_mem.blindsigning;
    nvm_write((void *)&N_nv_mem.blindsigning, (void *)&value, sizeof(uint8_t));
}
