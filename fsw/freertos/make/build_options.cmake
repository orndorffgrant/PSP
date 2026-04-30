##########################################################################
#
# Build options for "freertos" PSP
# This file specifies any global-scope compiler options when using this PSP
#
##########################################################################


set(INSTALL_SUBDIR "cf")
set(CFE_PSP_EXPECTED_OSAL_BSPTYPE "esp-idf-freertos")

add_definitions("-DESP_PLATFORM")
add_definitions("-DESP32S3=1")

if(NOT DEFINED IDF_PATH)
    message(FATAL_ERROR "IDF_PATH is not set. Use toolchain-xtensa-esp32s3-espidf.cmake.")
endif()

if(NOT DEFINED IDF_BUILD_DIR)
    message(FATAL_ERROR
        "IDF_BUILD_DIR is not set. Point it at the build/config/ directory "
        "from your ESP-IDF project.")
endif()

# Load the ESP-IDF CMake API (function definitions only — no targets created yet).
include("${IDF_PATH}/tools/cmake/idf.cmake")

# ---------------------------------------------------------------------------
# OSAL_LINK_LIBS — idf:: targets are created by idf_build_process() in the
# hook below; cmake resolves these references at generate time.
# ---------------------------------------------------------------------------
set(OSAL_LINK_LIBS
    idf::freertos
    idf::esp_system
    idf::esp_libc
    idf::log
    idf::esp_timer
)

# Resolve paths from IDF_BUILD_DIR (set by toolchain file)
get_filename_component(IDF_PROJECT_BUILD_DIR "${IDF_BUILD_DIR}" DIRECTORY)
get_filename_component(IDF_PROJECT_DIR       "${IDF_PROJECT_BUILD_DIR}" DIRECTORY)

set(IDF_SDKCONFIG "${IDF_PROJECT_DIR}/sdkconfig")
if(NOT EXISTS "${IDF_SDKCONFIG}")
    message(FATAL_ERROR
        "sdkconfig not found at ${IDF_SDKCONFIG}.")
endif()

message(STATUS "ESP-IDF idf_build_process: target=esp32s3")
message(STATUS "ESP-IDF sdkconfig: ${IDF_SDKCONFIG}")
message(STATUS "ESP-IDF build dir: ${IDF_PROJECT_BUILD_DIR}")

# Run idf_build_process in the same directory scope as add_executable().
# BUILD_DIR uses a subdirectory so ESP-IDF generated files don't collide
# with cFS's own generated files.
idf_build_process(esp32s3
    COMPONENTS
        freertos       # FreeRTOS kernel (ESP-IDF SMP fork)
        esp_system     # esp_restart(), etc.
        esp_timer      # esp_timer_get_time(), etc.
        log            # ESP_LOGx macros
        esp_libc       # libc (ESP-IDF patched newlib port)
    SDKCONFIG         "${IDF_SDKCONFIG}"
    SDKCONFIG_DEFAULTS ""
    PROJECT_DIR       "${IDF_PROJECT_DIR}"
    BUILD_DIR         "${CMAKE_BINARY_DIR}/idf_build"
)
