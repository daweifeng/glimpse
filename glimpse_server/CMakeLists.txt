cmake_minimum_required(VERSION 3.25)

project(glimpse_server CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

find_package(boost_uuid CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

find_path(VCPKG_INCLUDE_DIR "uwebsockets/App.h")
find_path(USOCKET_LIB_A "lib/libuSockets.a")



set(SOURCE_FILE
    src/main.cpp
    src/controller.cpp
    src/ws_manager.cpp
    src/room_manager.cpp
    src/room.cpp
)

add_executable(main
    ${SOURCE_FILE}
)

# uWebSockets has a macro that controls whether to write a mark to the response
add_compile_definitions(
    "UWS_HTTPRESPONSE_NO_WRITEMARK=1"
)

target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)
target_compile_features(main PRIVATE cxx_std_20)
# uwebsockets depends on uSockets and zlib, so we need to link it and include its headers
# libuSockets does not have a CMake config file, so we need to find it manually
target_include_directories(main PRIVATE "${VCPKG_INCLUDE_DIR}" "${VCPKG_INCLUDE_DIR}/uwebsockets")
target_link_libraries(main
    "${USOCKET_LIB_A}/lib/libuSockets.a"
    fmt::fmt
    spdlog::spdlog
    ZLIB::ZLIB
    Boost::uuid
    nlohmann_json::nlohmann_json
)
