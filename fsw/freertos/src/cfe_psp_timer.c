#include "cfe_psp.h"
#include "cfe_psp_config.h"
#include "cfe_psp_memory.h"


void CFE_PSP_GetTime(OS_time_t *time){
    time->ticks = esp_timer_get_time()*10;
    //multiply by 10 because esp uses microseconds and cfs wants 100 nanoseconds
}

void CFE_PSP_Get_Timebase(uint32 *tbu, uint32 *tbl){
    uint64_t total_ticks = (uint64_t)esp_timer_get_time() * 10;

    //split into upper 32 bits and lower 32 bits
    *tbu = (uint32)(total_ticks >> 32);
    *tbl = (uint32)(total_ticks & 0xFFFFFFFF);
}
