file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*
)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc PROPERTIES AUTORCC ON)

add_executable(ElmTerminalApp ${SOURCES})

target_link_libraries(ElmTerminalApp
    Qt6::Core
    Qt6::Widgets
    Qt6::SerialPort
)

target_include_directories(ElmTerminalApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# resources/styles.qrc is added to SOURCES above so it will be processed by AUTORCC

if(WIN32)
    add_custom_command(TARGET ElmTerminalApp POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
                "$<TARGET_FILE:ElmTerminalApp>"
        COMMENT "Running windeployqt..."
    )
endif()
