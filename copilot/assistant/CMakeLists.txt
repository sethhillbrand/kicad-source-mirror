find_package(wxWidgets CONFIG REQUIRED)
find_package(websocketpp CONFIG REQUIRED)

add_library( ${ASSISTANT_DLL_NAME} SHARED 
    assistant_launcher.cpp
    chat/chat_history.cpp
    chat/chat_panel_base.cpp
    chat/chat_panel.cpp  
    data_buried_point/fire_data_buried_point.cpp
    settings/copilot_settings_manager.cpp  
    websocket/websocket_endpoint.cpp
    websocket/websocket_event.cpp
    websocket/websocket_worker.cpp
    copilot_version.cpp
)

target_link_libraries(${ASSISTANT_DLL_NAME} PRIVATE 
    nlohmann_json
    ${wxWidgets_LIBRARIES}
    websocketpp::websocketpp
)

target_compile_definitions(${ASSISTANT_DLL_NAME} PRIVATE COPILOT_SHARED_LIBRARY)

install( TARGETS ${ASSISTANT_DLL_NAME}
    DESTINATION ${KICAD_BIN}/copilot
    COMPONENT binary
)