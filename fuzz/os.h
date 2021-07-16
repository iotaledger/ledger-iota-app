#pragma once

#define os_memcpy memcpy
#define os_memcmp memcmp
#define os_memset memset 
#define explicit_bzero(addr, size) memset((addr), 0, (size))

