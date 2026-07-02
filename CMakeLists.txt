cmake_minimum_required(VERSION 3.16)
project(kwin_contextmenudrag)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

# KWin's dependency
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(KF6 REQUIRED COMPONENTS CoreAddons Config)
find_package(KWin REQUIRED)

# Compile the plugin binary using KWin internal private header rules
add_compile_definitions(KWIN_EFFECT_API_MAKE_EXPORT_TREATMENT_HAPPIER)
add_library(kwin_contextmenudrag MODULE contextmenudragplugin.cpp)

# Ensure the compiler can find the generated kwin_contextmenudrag.json
target_include_directories(kwin_contextmenudrag PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(kwin_contextmenudrag PRIVATE Qt6::Gui KF6::CoreAddons KF6::ConfigCore KWin::kwin)

# Explicitly locate and link the KWin include directory
find_path(KWIN_ACTUAL_INCLUDE_DIR NAMES kwin_export.h PATH_SUFFIXES kwin)
if (KWIN_ACTUAL_INCLUDE_DIR)
    target_include_directories(kwin_contextmenudrag PRIVATE ${KWIN_ACTUAL_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Could not find KWin headers. Is kwin-devel fully installed?")
endif()
install(TARGETS kwin_contextmenudrag DESTINATION /usr/lib64/qt6/plugins/kwin/plugins)
