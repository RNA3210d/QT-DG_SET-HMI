cmake_minimum_required(VERSION 3.16)
project(DG_SET VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt — unchanged
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts 3DCore 3DRender 3DExtras 3DInput)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts 3DCore 3DRender 3DExtras 3DInput)

# KF6 — separate find_package, not mixed into the Qt one
find_package(KF6WidgetsAddons REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DG_SET
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        res.qrc
        reportdialog.h reportdialog.cpp
    )
else()
    if(ANDROID)
        add_library(DG_SET SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(DG_SET
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# target_link_libraries AFTER the executable is defined
target_link_libraries(DG_SET PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::3DCore
    Qt${QT_VERSION_MAJOR}::3DRender
    Qt${QT_VERSION_MAJOR}::3DExtras
    Qt${QT_VERSION_MAJOR}::3DInput
    KF6::WidgetsAddons
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.DG_SET)
endif()

set_target_properties(DG_SET PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS DG_SET
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DG_SET)
endif()