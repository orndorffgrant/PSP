#ifndef _cfe_psp_config_
#define _cfe_psp_config_

// FreeRTOS headers
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

// Memory tables
// cfe_psp_memory.c
#define CFE_PSP_MEMALIGN_MASK ((cpuaddr) 0x40)  // ARM may be designed with 64 byte processor cache
#define CFE_PSP_MEM_TABLE_SIZE 10
typedef struct
{
    uint32 bsp_reset_type;
    uint32 reserved1;
    uint32 reserved2;
    uint32 reserved3;
} CFE_PSP_ReservedMemoryBootRecord_t;


// Exception handling
// @FIXME needs to be implemented with cfe_psp_exception.c
#define CFE_PSP_MAX_EXCEPTION_ENTRIES 4
typedef UBaseType_t CFE_PSP_Exception_SysTaskId_t;
typedef struct
{
    uint32 reserved1;
} CFE_PSP_Exception_ContextDataEntry_t;

// Watchdog
// @FIXME needs to be implemented with cfe_psp_watchdog.c
#define CFE_PSP_WATCHDOG_MIN 0  // millieconds
#define CFE_PSP_WATCHDOG_MAX 0xFFFFFFFF  // milliseconds

// Miscellenous
#define CFE_PSP_NUM_EEPROM_BANKS 0

#endif

