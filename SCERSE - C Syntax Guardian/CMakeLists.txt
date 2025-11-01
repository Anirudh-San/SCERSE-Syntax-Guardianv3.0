#============================================================================
# FILE 8: CMakeLists.txt (UPDATED & WORKING)
#============================================================================

cmake_minimum_required(VERSION 3.16)
project(SCERSE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== CRITICAL: MSVC C++17 Support =====
if(MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

# ===== Qt Configuration =====
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# ===== Source Files =====
set(SOURCES
    main.cpp
    MainWindow.hpp
    MainWindow.cpp
    CodeEditor.hpp
    CodeEditor.cpp
    SyntaxHighlighter.hpp
    SyntaxHighlighter.cpp
)

# ===== Executable =====
add_executable(SCERSE ${SOURCES})

# ===== Link Libraries =====
target_link_libraries(SCERSE PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# ===== Compiler Warnings =====
if(MSVC)
    target_compile_options(SCERSE PRIVATE /W4)
else()
    target_compile_options(SCERSE PRIVATE -Wall -Wextra)
endif()
