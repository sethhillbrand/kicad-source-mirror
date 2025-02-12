set( COPILOT_CONFIG_DIR "copilot" CACHE STRING "Location of user specific copilot config files" )
mark_as_advanced( COPILOT_CONFIG_DIR )
add_compile_definitions( COPILOT_CONFIG_DIR=${COPILOT_CONFIG_DIR} )

# Generate version header file.
configure_file( ${PROJECT_SOURCE_DIR}/cmake/copilot_config.h.cmake
    ${CMAKE_BINARY_DIR}/copilot_config.h )


include_directories( BEFORE ${INC_BEFORE} )
include_directories(
    ./interface      
    .
    ${INC_AFTER}
)
add_subdirectory(assistant)