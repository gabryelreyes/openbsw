add_library(freeRtosCm7SysTickPort INTERFACE)

target_include_directories(freeRtosCm7SysTickPort INTERFACE include)

add_library(freeRtosCm7SysTick src/port.c)

target_include_directories(freeRtosCm7SysTick PUBLIC include)

target_link_libraries(
    freeRtosCm7SysTick
    PUBLIC freeRtos
    PRIVATE bspInterrupts)

target_compile_options(freeRtosCm7SysTick PRIVATE -Wno-unused-but-set-variable)
