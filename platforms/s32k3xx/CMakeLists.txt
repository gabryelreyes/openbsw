if (NOT BUILD_EXECUTABLE STREQUAL "unitTest")
    if (NOT DEFINED S32K3_CHIP)
        message(FATAL_ERROR "S32K3_CHIP must be defined (e.g., S32K344)")
    endif ()

    if (S32K3_CHIP STREQUAL "S32K344")
        include(${CMAKE_CURRENT_LIST_DIR}/cmake/s32k344.cmake)
    else ()
        message(
            FATAL_ERROR
                "Unsupported S32K3_CHIP: ${S32K3_CHIP}. Supported: S32K344")
    endif ()

    # Rtos libs depending on target rtos.
    if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
        add_subdirectory(3rdparty/freertos_cm7_sysTick)
    endif ()

    # BSP modules
    add_subdirectory(bsp)

    # ETL implementation
    add_subdirectory(etlImpl)
endif ()
