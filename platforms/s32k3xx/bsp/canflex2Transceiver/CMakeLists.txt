add_library(canflex2Transceiver
            src/can/transceiver/canflex2/CanFlex2Transceiver.cpp)

target_include_directories(canflex2Transceiver PUBLIC include)

target_link_libraries(canflex2Transceiver PUBLIC async bspFlexCan lifecycle)
