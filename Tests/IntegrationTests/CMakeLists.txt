#===================== Integration tests configuration ========================#
# Integration tests of Gpio module on target (Nucleo-144 / 32F746GDISCOVERY). Registered in module
# CMakeLists.txt by IntegrationTesting_AddPath() and created by
# IntegrationTesting_Generate() in "IntegrationTest" build
# (see EmBi_Platform/CMake/IntegrationTesting/README.md).
#
# The test set is independent - target interface (entry point, initialization,
# fault handlers, system reset) is implemented by ItTarget_Gpio.c and
# all libraries of the firmware are listed here.

# StartUp calls BspMain() - the entry point of the test firmware is provided by
# the test set (BspMain.h, ItTarget_Gpio.c)
if(NOT TARGET BspMain_Lib)
    add_library(BspMain_Lib INTERFACE)
    target_include_directories(BspMain_Lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(MCU_FAMILY_ID STREQUAL "STM32H7RSxx")

    # STM32H7R / STM32H7S (NUCLEO-H7S3L8)
    IntegrationTesting_Add_Test(
        NAME            Gpio
        TEST_SOURCE     ItTest_Gpio_H7RS.c
        SOURCES         ItTarget_Gpio.c     # Target interface of the test set
        LINK_LIBS       StartUp_Lib         # Startup code (calls BspMain)
                        Nvic_Lib            # Fault handlers, system reset (ItTarget)
                        Gpio_Lib
        TIMEOUT         30
    )

else()

    # Classic STM32H7 (NUCLEO-H723ZG, NUCLEO-H7A3ZI-Q)
    IntegrationTesting_Add_Test(
        NAME            Gpio
        TEST_SOURCE     ItTest_Gpio.c
        SOURCES         ItTarget_Gpio.c     # Target interface of the test set
        LINK_LIBS       StartUp_Lib         # Startup code (calls BspMain)
                        Nvic_Lib            # Fault handlers, system reset (ItTarget)
                        Gpio_Lib            # Module under test
        TIMEOUT         30
    )

endif()
