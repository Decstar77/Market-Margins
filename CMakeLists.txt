cmake_minimum_required(VERSION 3.20)
project(Finance)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define paths
set(JSON_DIR vendor/json)
set(SPD_LOG vendor/spdlog)
set(WEBSOCKETPP_DIR vendor/websocketpp)
set(SHARED_DIR shared)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Global compile options
add_compile_options(
    /MP  # MultiProcessorCompile
    /WX  # Treat warnings as errors
)

# Disable specific warnings on MSVC
if (MSVC)
    add_compile_options(
        /wd4057 /wd4100 /wd4152 /wd4200 /wd4201 /wd4204
        /wd4206 /wd4214 /wd4221 /wd4702 /wd4996
    )
endif()

# Common settings function
function(configure_target target_name)
    target_include_directories(${target_name} PRIVATE
        ${PROJECT_SOURCE_DIR}/${target_name}/${JSON_DIR}
        ${PROJECT_SOURCE_DIR}/${target_name}/${SPD_LOG}/include
        ${PROJECT_SOURCE_DIR}/${SHARED_DIR}
    )

    if (MSVC)
        target_link_libraries(${target_name} PRIVATE kernel32 user32 ws2_32 Dbghelp)
    endif()
endfunction()

# === Project: market ===
file(GLOB_RECURSE MARKET_SOURCES
    market/src/*.cpp
    market/src/*.c
    market/src/*.h
    market/src/*.hpp
)

add_executable(market ${MARKET_SOURCES})

# Precompiled header
target_precompile_headers(market PRIVATE market/src/fin-pch.cpp)

configure_target(market)

# === Project: desk-client ===
file(GLOB_RECURSE DESK_CLIENT_SOURCES
    desk-client/src/*.cpp
    desk-client/src/*.c
    desk-client/src/*.h
    desk-client/src/*.hpp
    ${SHARED_DIR}/*.cpp
)

add_executable(desk-client ${DESK_CLIENT_SOURCES})

configure_target(desk-client)

# === Build types ===
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Debug configuration
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG /Zi /Od")

# Release configuration
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /D_RELEASE /O2")
