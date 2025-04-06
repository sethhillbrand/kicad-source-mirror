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
    RUNTIME DESTINATION ${KICAD_LIB}
    LIBRARY DESTINATION ${KICAD_LIB}
    COMPONENT binary
)

if( APPLE )
    # puts library into the main kicad.app bundle in build tree
    set_target_properties( ${ASSISTANT_DLL_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${OSX_BUNDLE_BUILD_LIB_DIR}"
        INSTALL_NAME_DIR "${OSX_BUNDLE_BUILD_LIB_DIR}"
    )
    set_target_properties( ${ASSISTANT_DLL_NAME} PROPERTIES INSTALL_RPATH
            "@executable_path/../Frameworks" )
    set_target_properties( ${ASSISTANT_DLL_NAME} PROPERTIES BUILD_WITH_INSTALL_RPATH 1 )
endif()
