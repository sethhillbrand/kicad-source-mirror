find_package(CURL REQUIRED)

add_executable( assistant_updater 
    assistant_updater.cpp
    main.cpp
)
target_link_libraries(assistant_updater PRIVATE 
    CURL::libcurl
)