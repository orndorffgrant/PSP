/*
 * Copyright 2021 Patrick Paul
 * SPDX-License-Identifier: Apache-2.0 AND (Apache-2.0 OR MIT-0)
 * See further attribution at end of this file.
 */

#include "cfe_psp.h"
#include "cfe_psp_config.h"
#include "cfe_psp_memory.h"

// target_config.h provides GLOBAL_CONFIGDATA object for CFE runtime settings
#include <target_config.h>

// memory regions
#define CFE_PSP_CDS_SIZE (GLOBAL_CONFIGDATA.CfeConfig->CdsSize)
#define CFE_PSP_RESET_AREA_SIZE (GLOBAL_CONFIGDATA.CfeConfig->ResetAreaSize)
#define CFE_PSP_USER_RESERVED_SIZE (GLOBAL_CONFIGDATA.CfeConfig->UserReservedSize)
#define CFE_PSP_RAM_DISK_SECTOR_SIZE (GLOBAL_CONFIGDATA.CfeConfig->RamDiskSectorSize)
#define CFE_PSP_RAM_DISK_NUM_SECTORS (GLOBAL_CONFIGDATA.CfeConfig->RamDiskTotalSectors)

// gross size of psp memory to be allocated
#define CFE_PSP_RESERVED_MEMORY_SIZE 200 * 1024

// memory record type sizes
#define CFE_PSP_BOOT_RECORD_SIZE (sizeof(CFE_PSP_ReservedMemoryBootRecord_t))
#define CFE_PSP_EXCEPTION_RECORD_SIZE (sizeof(CFE_PSP_ExceptionStorage_t))


// cfe_psp_memory.h defines this type
CFE_PSP_ReservedMemoryMap_t CFE_PSP_ReservedMemoryMap = { 0 }; 

// allocate memory in a special memory region named ".psp_reserved" in linker script
// @FIXME determine whether to place CDS, other regions in NVM or other memory
__attribute__ ((section(".psp_reserved")))
__attribute__ ((aligned (8)))
char pspReservedMemoryAlloc[CFE_PSP_RESERVED_MEMORY_SIZE];


// zero-initialize certain memory depending on the reset type
int32 CFE_PSP_InitProcessorReservedMemory(uint32 reset_type){
    // @FIXME not implemented yet
    // memory may persist or be zero-initialized depending on linker memory region .psp_reserved
    return CFE_PSP_SUCCESS;
}

// assign the pointers for these memory regions:
// Boot Record, Exception Logging, Reset Reason, CDS, Volatile Disk, User Reserved
void CFE_PSP_SetupReservedMemoryMap(void){
    cpuaddr ReservedMemoryAddr;

    size_t ResetSize;
    size_t CDSSize;
    size_t UserReservedSize;
    size_t VolatileDiskSize;
    size_t RequiredSize;

    ResetSize = CFE_PSP_RESET_AREA_SIZE;
    VolatileDiskSize = (CFE_PSP_RAM_DISK_SECTOR_SIZE * CFE_PSP_RAM_DISK_NUM_SECTORS);
    CDSSize = CFE_PSP_CDS_SIZE;
    UserReservedSize = CFE_PSP_USER_RESERVED_SIZE;

    ResetSize = (ResetSize + CFE_PSP_MEMALIGN_MASK) & ~CFE_PSP_MEMALIGN_MASK;
    CDSSize = (CDSSize + CFE_PSP_MEMALIGN_MASK) & ~CFE_PSP_MEMALIGN_MASK;
    VolatileDiskSize = (VolatileDiskSize + CFE_PSP_MEMALIGN_MASK) & ~CFE_PSP_MEMALIGN_MASK;
    UserReservedSize = (UserReservedSize + CFE_PSP_MEMALIGN_MASK) & ~CFE_PSP_MEMALIGN_MASK;

    // calculate the required size and warn if not able to malloc
    RequiredSize = 0;
    RequiredSize += ResetSize;
    RequiredSize += VolatileDiskSize;
    RequiredSize += CDSSize;
    RequiredSize += UserReservedSize;
    OS_DebugPrintf("PSP reserved memory = %u bytes\n", (unsigned int) RequiredSize);
    if((unsigned int) RequiredSize > CFE_PSP_RESERVED_MEMORY_SIZE){
        OS_DebugPrintf("Not enough memory available for PSP CFE reserved sections.\n");
        return;
    }

    ReservedMemoryAddr = pspReservedMemoryAlloc;

    CFE_PSP_ReservedMemoryMap.BootPtr = (void*) ReservedMemoryAddr;
    ReservedMemoryAddr += CFE_PSP_BOOT_RECORD_SIZE;

    CFE_PSP_ReservedMemoryMap.ExceptionStoragePtr = (void*) ReservedMemoryAddr;
    ReservedMemoryAddr += CFE_PSP_EXCEPTION_RECORD_SIZE;

    CFE_PSP_ReservedMemoryMap.ResetMemory.BlockPtr = (void*) ReservedMemoryAddr;
    CFE_PSP_ReservedMemoryMap.ResetMemory.BlockSize = CFE_PSP_RESET_AREA_SIZE;
    ReservedMemoryAddr += ResetSize;

    CFE_PSP_ReservedMemoryMap.VolatileDiskMemory.BlockPtr = (void*) ReservedMemoryAddr;
    CFE_PSP_ReservedMemoryMap.VolatileDiskMemory.BlockSize = CFE_PSP_RAM_DISK_SECTOR_SIZE * CFE_PSP_RAM_DISK_NUM_SECTORS;
    ReservedMemoryAddr += VolatileDiskSize;

    CFE_PSP_ReservedMemoryMap.CDSMemory.BlockPtr = (void*) ReservedMemoryAddr;
    CFE_PSP_ReservedMemoryMap.CDSMemory.BlockSize = CFE_PSP_CDS_SIZE;
    ReservedMemoryAddr += CDSSize;

    CFE_PSP_ReservedMemoryMap.UserReservedMemory.BlockPtr = (void*) ReservedMemoryAddr;
    CFE_PSP_ReservedMemoryMap.UserReservedMemory.BlockSize = CFE_PSP_USER_RESERVED_SIZE;
    ReservedMemoryAddr += UserReservedSize;
}

/*
**
**  This file is derived from these files:
**
**      mcp750-vxworks/src/cfe_psp_memory.c
**      pc-linux/src/cfe_psp_memory.c
**      pc-rtems/src/cfe_psp_memory.c
**
**  These works are licensed under the following terms:
**
**      GSC-18128-1, "Core Flight Executive Version 6.7"
**      
**      Copyright (c) 2006-2019 United States Government as represented by
**      the Administrator of the National Aeronautics and Space Administration.
**      All Rights Reserved.
**      
**      Licensed under the Apache License, Version 2.0 (the "License");
**      you may not use this file except in compliance with the License.
**      You may obtain a copy of the License at
**      
**        http://www.apache.org/licenses/LICENSE-2.0
**      
**      Unless required by applicable law or agreed to in writing, software
**      distributed under the License is distributed on an "AS IS" BASIS,
**      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**      See the License for the specific language governing permissions and
**      limitations under the License.
**
**  Modifications in this file authored by Patrick Paul are available under either the Apache-2.0 or MIT-0 license.
**
*/