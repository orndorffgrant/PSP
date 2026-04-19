include_directories(inc)

add_library(psp-${CFE_PSP_TARGETNAME}-impl OBJECT
    src/cfe_psp_memory.c
    src/cfe_psp_start.c
    src/cfe_psp_timer.c
    vendor/cfe_psp_memory.c
    vendor/cfe_psp_memtab.c
    todo/cfe_psp_exception.c
    todo/cfe_psp_memory.c
    todo/cfe_psp_support.c
    todo/cfe_psp_timer.c
)
