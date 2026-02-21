cmake_minimum_required(VERSION 3.20)
project(MyMod VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

# Use vcpkg toolchain if available
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_library(noclip MODULE
    src/my_mod.cpp
    src/noclip.cpp
    src/keybind.cpp
    src/teleport_list.cpp
)

target_link_libraries(noclip PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/winhttp.lib
)

target_include_directories(noclip PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(noclip PROPERTIES
    PREFIX ""
    SUFFIX ".dll"
)
