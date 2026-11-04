add_library(
    bspConfiguration
    src/bsp/stdIo/stdIo.cpp src/bsp/SystemTimer/SystemTimer.cpp
    src/bsp/uart/UartConfig.cpp)

target_include_directories(bspConfiguration PUBLIC include)

target_link_libraries(bspConfiguration PUBLIC bsp bspInterrupts bspMcu bspUart)
