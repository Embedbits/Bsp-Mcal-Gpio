#======================== Unit tests configuration ============================#
# Unit tests of Gpio module. Registered in module CMakeLists.txt by
# UnitTesting_AddPath() and created by UnitTesting_Generate() in "UnitTest"
# build (see EmBi_Platform/CMake/UnitTesting/README.md).

get_filename_component(GPIO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
get_filename_component(MCAL_DIR "${GPIO_DIR}/.." ABSOLUTE)

UnitTesting_Add_Test(
    NAME            Gpio
    TEST_SOURCE     Test_Gpio.c
    UUT_SOURCES     ${GPIO_DIR}/Gpio.c
    MOCK_HEADERS    ${MCAL_DIR}/Rcc/Rcc_Port.h
    INCLUDE_DIRS    ${GPIO_DIR}
                    ${MCAL_DIR}/Rcc
    LINK_LIBS       Ral_Lib
)
