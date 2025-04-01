find_package( wxWidgets ${wxWidgets_REQ_VERSION} COMPONENTS  core base webview REQUIRED )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
add_library( ${ASSISTANT_DLL_NAME} SHARED 
    assistant_launcher.cpp
    settings/copilot_settings_manager.cpp  
    webview/webview_container.cpp
    http/http_client.cpp
    copilot_version.cpp
    copilot.rc
)

target_include_directories( ${ASSISTANT_DLL_NAME} PRIVATE
    $<TARGET_PROPERTY:magic_enum,INTERFACE_INCLUDE_DIRECTORIES>
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