#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "common_types.h"
#include "osapi.h"
#include "osapi-os-core.h"
#include "osapi-os-filesys.h"


int32 CFE_PSP_AttachExceptions(void){
    return -1;
}

int32 CFE_PSP_ExceptionGetSummary_Impl(void){
    return -1;
}

int32 CFE_PSP_SetDefaultExceptionEnvironment(void){
    return -1;
}
