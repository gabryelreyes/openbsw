add_subdirectory(bspConfiguration)
add_subdirectory(safety)
add_subdirectory(main)

if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    add_subdirectory(freeRtosCoreConfiguration)
    add_library(freeRtosPort ALIAS freeRtosCm7SysTickPort)
    add_library(freeRtosPortImpl ALIAS freeRtosCm7SysTick)
endif ()
