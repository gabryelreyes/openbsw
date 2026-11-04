add_library(
    bspConfiguration
    src/bsp/adc/AnalogInput.cpp src/bsp/stdIo/stdIo.cpp
    src/bsp/SystemTimer/SystemTimer.cpp src/bsp/uart/UartConfig.cpp)

target_include_directories(bspConfiguration PUBLIC include)

target_link_libraries(
    bspConfiguration
    PUBLIC bsp
           bspAdc
           bspInterrupts
           bspIo
           bspMcu
           bspOutputPwm
           bspUart)
