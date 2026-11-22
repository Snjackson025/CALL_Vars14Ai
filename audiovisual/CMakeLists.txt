cmake_minimum_required(VERSION 3.21)
project(CallVarsAudiovisual LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Multimedia WebSockets Network)

add_executable(CallVars
    main.cpp
    CallVarsWindow.h
    CallVarsWindow.cpp
    FoxCore.h
    FoxCore.cpp
)

target_link_libraries(CallVars PRIVATE
    Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::WebSockets Qt6::Network
)

if(WIN32)
    target_compile_definitions(CallVars PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()
