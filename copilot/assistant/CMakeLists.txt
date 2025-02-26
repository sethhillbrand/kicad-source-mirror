find_package(wxWidgets CONFIG REQUIRED)
find_package(websocketpp CONFIG REQUIRED)

add_library( assistant SHARED 
    assistant_launcher.cpp
    chat/chat_context.cpp
    chat/chat_history.cpp
    chat/chat_panel_base.cpp
    chat/chat_panel.cpp
    copilot_version.cpp
)

target_link_libraries(assistant PRIVATE 
    nlohmann_json
    ${wxWidgets_LIBRARIES}
    websocketpp::websocketpp
)

target_compile_definitions(assistant PRIVATE COPILOT_SHARED_LIBRARY)

install( TARGETS assistant
    DESTINATION ${KICAD_BIN}
    COMPONENT binary
)