find_package(wxWidgets CONFIG REQUIRED)

add_executable( copilot_standalone  WIN32
    main.cpp
    sample.rc
    wx.manifest
)

if (WXMSW AND DEFINED wxUSE_DPI_AWARE_MANIFEST)
set(wxUSE_DPI_AWARE_MANIFEST_VALUE 0)
if (${wxUSE_DPI_AWARE_MANIFEST} MATCHES "system")
    set(wxUSE_DPI_AWARE_MANIFEST_VALUE 1)
    list(APPEND src_files "${wxSOURCE_DIR}/include/wx/msw/wx_dpi_aware.manifest")
elseif(${wxUSE_DPI_AWARE_MANIFEST} MATCHES "per-monitor")
    set(wxUSE_DPI_AWARE_MANIFEST_VALUE 2)
    list(APPEND src_files "${wxSOURCE_DIR}/include/wx/msw/wx_dpi_aware_pmv2.manifest")
endif()
endif()

target_link_libraries(copilot_standalone PRIVATE 
    ${wxWidgets_LIBRARIES}
    nlohmann_json
)

install( TARGETS copilot_standalone
    DESTINATION ${KICAD_BIN}
    COMPONENT binary
)