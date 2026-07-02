if (NOT BUILD_EXECUTABLE STREQUAL "unitTest")
    add_subdirectory(bspClock)
    add_subdirectory(bspInterruptsImpl)
    add_subdirectory(bspMcu)
    add_subdirectory(bspUart)
endif ()

add_library(socBsp INTERFACE)

if (NOT BUILD_EXECUTABLE STREQUAL "unitTest")
    target_link_libraries(socBsp INTERFACE bspClock bspInterruptsImpl bspMcu
                                           bspUart)
endif ()
