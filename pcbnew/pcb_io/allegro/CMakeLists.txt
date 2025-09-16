# Sources for the pcbnew PLUGIN called ALLEGRO_PLUGIN

set( ALLEGRO2PCBNEW_SRCS
    allegro_parser_pcb.cpp
    allegro_file.cpp
    pcb_io_allegro.cpp
    )

# Create a static library for the Allegro parser
add_library( allegro2pcbnew STATIC ${ALLEGRO2PCBNEW_SRCS} )

# Link required libraries
target_link_libraries( allegro2pcbnew
    pcbcommon
    )

# Public include directories
target_include_directories( allegro2pcbnew
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

# Private include directories (if any dependencies need them)
target_include_directories( allegro2pcbnew PRIVATE
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/include
    )

# Compiler definitions
target_compile_definitions( allegro2pcbnew
    PRIVATE
        PCBNEW
    )