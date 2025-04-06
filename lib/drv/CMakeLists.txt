cmake_minimum_required(VERSION 3.13)

include(./../../pico_sdk_import.cmake)

project(drv C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_library(${PROJECT_NAME} 
    src/drv824xh.cpp
    src/drv824xs.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src )

target_link_libraries(${PROJECT_NAME}
  pico_stdlib
  hardware_pwm
  hardware_spi
)