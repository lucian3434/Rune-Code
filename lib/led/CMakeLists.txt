cmake_minimum_required(VERSION 3.13)

include(./../../pico_sdk_import.cmake)

project(led C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_library(${PROJECT_NAME} src/ws2812.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src )

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)

target_link_libraries(${PROJECT_NAME}
  pico_stdlib
  hardware_pio
)