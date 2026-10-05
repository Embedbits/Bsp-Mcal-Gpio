#===================== Integration tests configuration ========================#
# Integration tests of Gpio module on target (Nucleo board). Registered in module
# CMakeLists.txt by IntegrationTesting_AddPath() and created by
# IntegrationTesting_Generate() in "IntegrationTest" build
# (see EmBi_Platform/CMake/IntegrationTesting/README.md).

IntegrationTesting_Add_Test(
    NAME            Gpio
    TEST_SOURCE     ItTest_Gpio.c
    LINK_LIBS       Gpio_Lib
    TIMEOUT         30
)
