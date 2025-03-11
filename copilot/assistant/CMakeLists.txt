find_package(wxWidgets CONFIG REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library( ${ASSISTANT_DLL_NAME} SHARED 
    assistant_launcher.cpp
    settings/copilot_settings_manager.cpp  
    webview/webview_container.cpp
    http/http_client.cpp
    copilot_version.cpp
)

target_link_libraries(${ASSISTANT_DLL_NAME} PRIVATE 
    nlohmann_json
    ${wxWidgets_LIBRARIES}
)

target_compile_definitions(${ASSISTANT_DLL_NAME} PRIVATE COPILOT_SHARED_LIBRARY)

install( TARGETS ${ASSISTANT_DLL_NAME}
    DESTINATION ${KICAD_BIN}/copilot
    COMPONENT binary
)