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

# Directory for public headers (build interface)
set(PUBLIC_HEADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/PublicHeaders)

file(MAKE_DIRECTORY ${PUBLIC_HEADERS_DIR})

# Process all public headers
foreach(PublicHeader IN LISTS Gpio_PublicHeaders)
    
    # Copy (or configure) only the public header there
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${PublicHeader} 
        ${PUBLIC_HEADERS_DIR}/${PublicHeader}
        COPYONLY
    )
    
endforeach()


# Include necessary libraries
target_link_libraries( Gpio_Lib 
    PUBLIC 
    ${Gpio_DependentLibs}
)


target_include_directories( Gpio_Lib 
    PUBLIC
        $<BUILD_INTERFACE:${PUBLIC_HEADERS_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${Rcc_IncludeDirs}
)


message(STATUS "Gpio_Lib library created")

#========================= Documentation configuration ========================#
if(DOXYGEN_AVAILABLE STREQUAL "ON")
    Doxygen_AddPath("${CMAKE_CURRENT_SOURCE_DIR}/")
endif()
