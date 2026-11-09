cmake_minimum_required(VERSION 3.19)
project(FaultMonitoringSystem LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(FaultMonitoringSystem
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    processlister.h processlister.cpp
    processmodel.h processmodel.cpp
)

target_link_libraries(FaultMonitoringSystem
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS FaultMonitoringSystem
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET FaultMonitoringSystem
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
