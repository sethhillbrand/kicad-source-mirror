add_library( copilot SHARED copilot.cpp )

target_compile_definitions(copilot PRIVATE COPILOT_SHARED_LIBRARY)