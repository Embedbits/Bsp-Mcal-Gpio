# Include current directory for provide headers include
set( Gpio_IncludeDirs
    ${CMAKE_CURRENT_SOURCE_DIR} 
)


# Set source files for library generation
# All used function files shall be added here.
set( Gpio_LibSrcs
    Gpio.c )


# Set public header files for the library
# All header files used by other libraries shall be added here.
set( Gpio_PublicHeaders 
    Gpio_Port.h 
)


# Set dependent libraries
# All libraries used by current target shall be added here.
set( Gpio_DependentLibs
    Ral_Lib
    Rcc_Lib
)


# Create library
add_library(Gpio_Lib STATIC ${Gpio_LibSrcs})


# Specify the public headers for the library
set_target_properties( Gpio_Lib PROPERTIES PUBLIC_HEADER "${Gpio_PublicHeaders}" )


# Include necessary libraries
target_link_libraries( Gpio_Lib 
    PRIVATE 
    PUBLIC
    ${Gpio_DependentLibs}
)


target_include_directories( Gpio_Lib 
    INTERFACE 
    ${Gpio_IncludeDirs}
)


message(STATUS "Gpio_Lib library created")

#========================= Documentation configuration ========================#
if(DOXYGEN_AVAILABLE STREQUAL "ON")
    Doxygen_AddPath("${CMAKE_CURRENT_SOURCE_DIR}/")
endif()
