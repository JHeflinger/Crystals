cmake_minimum_required(VERSION 3.5)

# Enable debug symbols by default, must be done before project() statement
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()
# (you can also set it on the command line: -D CMAKE_BUILD_TYPE=Release)

set(EXECUTABLE_NAME "spectrum")
project(${EXECUTABLE_NAME})

find_package(Vulkan REQUIRED)

add_definitions ( -DEXEC_NAME="${EXECUTABLE_NAME}" )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

file(GLOB_RECURSE HEADERS "src/**.h")
file(GLOB_RECURSE SOURCES_CPP "src/**.cpp")
file(GLOB_RECURSE SOURCES_C "src/**.c")

set( SOURCES ${SOURCES_CPP} ${SOURCES_C} )
add_executable(${EXECUTABLE_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${EXECUTABLE_NAME} PRIVATE
    src
    include
    vendor
)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

if(UNIX AND NOT APPLE)
    set(LIBS ${LIBS} ${CMAKE_DL_LIBS})
endif()

target_link_libraries(${EXECUTABLE_NAME} PRIVATE Vulkan::Vulkan ${LIBS})


