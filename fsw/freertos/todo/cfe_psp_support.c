#include "cfe_psp.h"
#include "cfe_psp_config.h"
#include "cfe_psp_memory.h"


extern uint32 CFE_PSP_SpacecraftId;
extern uint32 CFE_PSP_CpuId;

uint32 CFE_PSP_GetProcessorId(void){
    return CFE_PSP_CpuId;
}

uint32 CFE_PSP_GetSpacecraftId(void){
    return CFE_PSP_SpacecraftId;
}

void CFE_PSP_Restart(uint32 resetType){
    if (restart_type == CFE_PSP_RST_TYPE_POWERON)
    {
        OS_printf("CFE_PSP: Exiting cFE with POWERON Reset status.\n");
        //TODO: something different should be done here, but idrk what exactly
    }
    else
    {
        OS_printf("CFE_PSP: Exiting cFE with PROCESSOR Reset Status. \n");
    }

    vTaskDelay(pdMS_TO_TICKS(100));
    esp_restart();
}
