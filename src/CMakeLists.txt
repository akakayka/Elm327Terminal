file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*
)

add_executable(Elm327Console ${SOURCES})

target_link_libraries(Elm327Console
    Qt6::Core
    Qt6::Widgets
    Qt6::SerialPort
)

target_include_directories(Elm327Console PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(WIN32)
    add_custom_command(TARGET Elm327Console POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
                "$<TARGET_FILE:Elm327Console>"
        COMMENT "Running windeployqt..."
    )
endif()