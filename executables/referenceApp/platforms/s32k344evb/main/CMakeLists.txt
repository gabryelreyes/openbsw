add_library(startUpAsm OBJECT src/bsp/bootHeader.S src/bsp/startUp.S)

set_target_properties(startUpAsm PROPERTIES COMPILE_WARNING_AS_ERROR OFF)

add_library(startUp "$<TARGET_OBJECTS:startUpAsm>" src/os/isr/isr_can.cpp)

set_target_properties(startUp PROPERTIES LINKER_LANGUAGE CXX)

add_library(main src/lifecycle/StaticBsp.cpp src/systems/CanSystem.cpp
                 src/main.cpp)

target_include_directories(main PUBLIC include)

target_link_libraries(
    main
    PRIVATE bsp
            bspClock
            bspConfiguration
            bspMcu
            canflex2Transceiver
            configuration
            etl
            lifecycle
            safeSupervisor
            startUp)

if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    target_link_libraries(main PRIVATE freeRtosCm7SysTick)
    add_library(osHooks src/osHooks/freertos/osHooks.cpp)
    target_include_directories(osHooks PRIVATE include)
    target_link_libraries(osHooks PRIVATE freeRtosCm7SysTick)
endif ()

set(LINKERSCRIPT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/linkerscript")
set(LINKERSCRIPT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/linkerscript")
set(GENERATED_LINKER_SCRIPT "${LINKERSCRIPT_BINARY_DIR}/application.dld")

add_custom_command(
    OUTPUT "${GENERATED_LINKER_SCRIPT}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${LINKERSCRIPT_BINARY_DIR}"
    COMMAND
        ${CMAKE_C_COMPILER} -E -P -x c -I "${LINKERSCRIPT_SOURCE_DIR}"
        "${LINKERSCRIPT_SOURCE_DIR}/application.dld.in" -o
        "${GENERATED_LINKER_SCRIPT}"
    DEPENDS "${LINKERSCRIPT_SOURCE_DIR}/application.dld.in"
    VERBATIM)

add_custom_target(generateStartUpLinkerScript
                  DEPENDS "${GENERATED_LINKER_SCRIPT}")

add_dependencies(startUp generateStartUpLinkerScript)

set_target_properties(startUp PROPERTIES PROP_LINKER_SCRIPT
                                         "${GENERATED_LINKER_SCRIPT}")
set_target_properties(startUp PROPERTIES LINK_DEPENDS
                                         "${GENERATED_LINKER_SCRIPT}")
