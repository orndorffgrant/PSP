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

#include <string.h>

#include "esp_attr.h"

// memory regions
#define CFE_PSP_CDS_SIZE (GLOBAL_CONFIGDATA.CfeConfig->CdsSize)
#define CFE_PSP_RESET_AREA_SIZE (GLOBAL_CONFIGDATA.CfeConfig->ResetAreaSize)
#define CFE_PSP_USER_RESERVED_SIZE (GLOBAL_CONFIGDATA.CfeConfig->UserReservedSize)
#define CFE_PSP_RAM_DISK_SECTOR_SIZE (GLOBAL_CONFIGDATA.CfeConfig->RamDiskSectorSize)
#define CFE_PSP_RAM_DISK_NUM_SECTORS (GLOBAL_CONFIGDATA.CfeConfig->RamDiskTotalSectors)

// gross size of psp memory to be allocated
#define CFE_PSP_RESERVED_MEMORY_SIZE (512 * 1024)

// memory record type sizes
#define CFE_PSP_BOOT_RECORD_SIZE (sizeof(CFE_PSP_ReservedMemoryBootRecord_t))
#define CFE_PSP_EXCEPTION_RECORD_SIZE (sizeof(CFE_PSP_ExceptionStorage_t))

// This is how regular FreeRTOS marks its code sections
// extern unsigned int _stext;
// extern unsigned int _etext; 
// #define CODE_START_ADDR ((cpuaddr)&_stext)
// #define CODE_END_ADDR ((cpuaddr)&_etext)

//This is how the esp-idf specific version of FreeRTOS marks its code section
// because unfortunately the regular FreeRTOS stuff above will have undefined behavior on eps-idf
extern unsigned int _instruction_reserved_start;
extern unsigned int _instruction_reserved_end;
#define CODE_START_ADDR ((cpuaddr)&_instruction_reserved_start)
#define CODE_END_ADDR ((cpuaddr)&_instruction_reserved_end)

// cfe_psp_memory.h defines this type
CFE_PSP_ReservedMemoryMap_t CFE_PSP_ReservedMemoryMap = { 0 }; 

// .psp_reserved not in the IDF linker script. need to add if this is necessary
// putting in psram instead
// allocate memory in a special memory region named ".psp_reserved" in linker script
// @FIXME determine whether to place CDS, other regions in NVM or other memory
// __attribute__ ((section(".psp_reserved")))
__attribute__ ((aligned (8)))
EXT_RAM_BSS_ATTR char pspReservedMemoryAlloc[CFE_PSP_RESERVED_MEMORY_SIZE];


// zero-initialize certain memory depending on the reset type
int32 CFE_PSP_InitProcessorReservedMemory(uint32 reset_type)
{
    if (reset_type == CFE_PSP_RST_TYPE_POWERON)
    {
        PSP_DEBUG("CFE_PSP: Clearing Processor Reserved Memory.\n");
        memset(CFE_PSP_ReservedMemoryMap.BootPtr, 0, sizeof(CFE_PSP_ReservedMemoryBootRecord_t));
        memset(CFE_PSP_ReservedMemoryMap.ExceptionStoragePtr, 0, sizeof(CFE_PSP_ExceptionStorage_t));
        memset(CFE_PSP_ReservedMemoryMap.ResetMemory.BlockPtr, 0, CFE_PSP_ReservedMemoryMap.ResetMemory.BlockSize);
        memset(CFE_PSP_ReservedMemoryMap.VolatileDiskMemory.BlockPtr, 0, CFE_PSP_ReservedMemoryMap.VolatileDiskMemory.BlockSize);
        memset(CFE_PSP_ReservedMemoryMap.CDSMemory.BlockPtr, 0, CFE_PSP_ReservedMemoryMap.CDSMemory.BlockSize);
        memset(CFE_PSP_ReservedMemoryMap.UserReservedMemory.BlockPtr, 0, CFE_PSP_ReservedMemoryMap.UserReservedMemory.BlockSize);

        CFE_PSP_ReservedMemoryMap.BootPtr->bsp_reset_type = CFE_PSP_RST_TYPE_PROCESSOR;
    }

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
    PSP_DEBUG("PSP reserved memory = %u bytes\n", (unsigned int) RequiredSize);
    if((unsigned int) RequiredSize > CFE_PSP_RESERVED_MEMORY_SIZE){
        PSP_DEBUG("Not enough memory available for PSP CFE reserved sections.\n");
        return;
    }

    ReservedMemoryAddr = (cpuaddr)pspReservedMemoryAlloc;

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

/*--------------------------------------------------------------------------------------*/
/**
 * @brief Returns the location and size of the kernel memory.
 *
 * This function returns the start and end address of the kernel text segment.
 * It may not be implemented on all architectures.
 *
 * @param[out] PtrToKernelSegment  Pointer to the variable that will store the location of the kernel text segment
 * @param[out] SizeOfKernelSegment Pointer to the variable that will store the size of the kernel text segment
 *
 * @return 0 (OS_SUCCESS or CFE_PSP_SUCCESS) on success, -1 (OS_ERROR or CFE_PSP_ERROR) on error
 * or CFE_PSP_ERROR_NOT_IMPLEMENTED if not implemented
 */
int32 CFE_PSP_GetKernelTextSegmentInfo(cpuaddr *PtrToCFESegment, uint32 *SizeOfCFESegment){
    int32 return_code;

    if (SizeOfCFESegment == NULL)
    {
        return_code = CFE_PSP_ERROR;
    }
    else
    {
        *PtrToCFESegment  = CODE_START_ADDR;
        *SizeOfCFESegment = (uint32)(CODE_END_ADDR - CODE_START_ADDR);

        return_code = CFE_PSP_SUCCESS;
    }

    return return_code;
}

/*--------------------------------------------------------------------------------------*/
/**
 * @brief Returns the location and size of the kernel memory.
 *
 * This function returns the start and end address of the CFE text segment. It
 * may not be implemented on all architectures.
 *
 * @param[out] PtrToCFESegment  Pointer to the variable that will store the location of the cFE text segment
 * @param[out] SizeOfCFESegment Pointer to the variable that will store the size of the cFE text segment
 *
 * @return 0 (OS_SUCCESS or CFE_PSP_SUCCESS) on success, -1 (OS_ERROR or CFE_PSP_ERROR) on error
 */
int32 CFE_PSP_GetCFETextSegmentInfo(cpuaddr *PtrToCFESegment, uint32 *SizeOfCFESegment)
{
    int32 return_code;
    return_code = CFE_PSP_ERROR;

    //i think the CFE_PSP_MAIN_FUNCTION macro works for the start point of our cfe code
    //but im not sure what would work as the end point

    return return_code;
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
