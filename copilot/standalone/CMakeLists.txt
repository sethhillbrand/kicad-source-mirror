find_package(wxWidgets CONFIG REQUIRED)

add_executable( copilot_standalone  WIN32
    main.cpp
)

target_link_libraries(copilot_standalone PRIVATE 
    ${wxWidgets_LIBRARIES}
)

install( TARGETS copilot_standalone
    DESTINATION ${KICAD_BIN}
    COMPONENT binary
)