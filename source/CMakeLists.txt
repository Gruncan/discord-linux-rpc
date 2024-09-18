include_directories(../include)
add_executable(
    discord-linux-rpc
        main.c
)

target_link_libraries(discord-linux-rpc discord-rpc)

install(
    TARGETS discord-linux-rpc
    RUNTIME
        DESTINATION "bin"
        CONFIGURATIONS Release
    BUNDLE
        DESTINATION "bin"
        CONFIGURATIONS Release
)