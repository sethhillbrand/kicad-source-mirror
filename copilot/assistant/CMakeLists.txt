find_package(wxWidgets CONFIG REQUIRED)
find_package(websocketpp CONFIG REQUIRED)

add_library( assistant SHARED 
    assistant_launcher.cpp
    chat/chat_history.cpp
    chat/chat_panel_base.cpp
    chat/chat_panel.cpp  
    settings/copilot_settings_manager.cpp  
    websocket/websocket_endpoint.cpp
    websocket/websocket_event.cpp
    websocket/websocket_worker.cpp
    copilot_version.cpp
)

target_link_libraries(assistant PRIVATE 
    nlohmann_json
    ${wxWidgets_LIBRARIES}
    websocketpp::websocketpp
)

target_compile_definitions(assistant PRIVATE COPILOT_SHARED_LIBRARY)

install( TARGETS assistant
    DESTINATION ${KICAD_BIN}/copilot
    COMPONENT binary
)