add_library( assistant SHARED 
    common/assistant_version.cpp
    settings/copilot_settings_manager.cpp  
    webview/webview_container.cpp
    http/http_client.cpp
)

target_include_directories( assistant
    PUBLIC
    .
    $<TARGET_PROPERTY:magic_enum,INTERFACE_INCLUDE_DIRECTORIES>
    )

target_link_libraries(assistant PRIVATE 
    nlohmann_json
    ${wxWidgets_LIBRARIES}
)

target_compile_definitions(assistant PRIVATE COPILOT_SHARED_LIBRARY)

install( TARGETS assistant
    RUNTIME DESTINATION ${KICAD_LIB}
    LIBRARY DESTINATION ${KICAD_LIB}
    COMPONENT binary
)

if( APPLE )
    # puts library into the main kicad.app bundle in build tree
    set_target_properties( assistant PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${OSX_BUNDLE_BUILD_LIB_DIR}"
        INSTALL_NAME_DIR "${OSX_BUNDLE_BUILD_LIB_DIR}"
    )
    set_target_properties( assistant PROPERTIES INSTALL_RPATH
            "@executable_path/../Frameworks" )
    set_target_properties( assistant PROPERTIES BUILD_WITH_INSTALL_RPATH 1 )
endif()
