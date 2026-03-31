# Template version: 0.0.1
# Sets the version of CMakeLists.txt file.
# Never, under any circumstances, change this line. It is used by
# the module generation script to identify the template version.

#======================== Library configuration ===============================#
# Publicly included directories to provide headers include
set( Gpio_PublicIncludeDirs
    
)


# Privately included directories to provide headers include
set( Gpio_PrivateIncludeDirs
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# Source files list for current library generation
set( Gpio_SourceFiles
    Gpio.c 
)


# List of public header files provided by the current library
set( Gpio_PublicHeaders 
    Gpio_Port.h 
)


# List of dependent libraries that are publicly used by the current library
set( Gpio_PublicDependentLibs
    Ral_Lib
Rcc_Lib 
)


# List of dependent libraries that are privately used by the current library
set( Gpio_PrivateDependentLibs
     
)


#========================== Library generation ================================#
# Create library
add_library(Gpio_Lib STATIC ${Gpio_SourceFiles})


# Include necessary libraries to the target
target_link_libraries( Gpio_Lib
    PUBLIC 
        ${Gpio_PublicDependentLibs} 
    PRIVATE 
        ${Gpio_PrivateDependentLibs}
)


# Set public headers directory path in build folder
set(PUBLIC_HEADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/PublicHeaders)

# Generates the public headers directory in build folder
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


# Set include directories for the created target
target_include_directories( Gpio_Lib
    PUBLIC
        $<BUILD_INTERFACE:${PUBLIC_HEADERS_DIR}>
        $<INSTALL_INTERFACE:include>
        Gpio_PublicIncludeDirs
    PRIVATE
        ${Gpio_IncludeDirs}
        Gpio_PrivateIncludeDirs
)


# Print status message
message(STATUS "Gpio_Lib library created")

#========================= Documentation configuration ========================#
if(DOXYGEN_AVAILABLE STREQUAL "ON")
    Doxygen_AddPath("${CMAKE_CURRENT_SOURCE_DIR}/")
endif()
