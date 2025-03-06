set( COPILOT_CONFIG_DIR "copilot" CACHE STRING "Location of user specific copilot config files" )
mark_as_advanced( COPILOT_CONFIG_DIR )
add_compile_definitions( COPILOT_CONFIG_DIR=${COPILOT_CONFIG_DIR} )

set( ASSISTANT_DLL_NAME "assistant" CACHE STRING "dll name of the entrypoint for copilot" )
mark_as_advanced( ASSISTANT_DLL_NAME )
add_compile_definitions( ASSISTANT_DLL_NAME=${ASSISTANT_DLL_NAME} )

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
add_subdirectory(updater)
add_subdirectory(standalone)