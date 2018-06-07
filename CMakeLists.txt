cmake_minimum_required(VERSION 2.8.4)
project(PahoTest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCES
        main/main.cpp
        )

add_executable(dont_build_this ${SOURCES})
target_include_directories(dont_build_this PUBLIC ../esp-idf/components/freertos/include/)
target_include_directories(dont_build_this PUBLIC ../esp-idf/components/esp32/include/)
target_include_directories(dont_build_this PUBLIC ../esp-idf/components/log/include/)
target_include_directories(dont_build_this PUBLIC ../esp-idf/components/nvs_flash/include/)
target_include_directories(dont_build_this PUBLIC ../esp-idf/components/lwip/include/lwip/)
target_include_directories(dont_build_this PUBLIC ../esp-idf/components/paho/FreeRtosPort/)
target_include_directories(dont_build_this PUBLIC ../esp-idf/components/paho/MQTTClient/src/)
target_include_directories(dont_build_this PUBLIC ../esp-idf/components/paho/MQTTPacket/src/)

add_custom_target(PahoTest COMMAND make IDF_PATH=/home/yesiot/workshop_esp32/esp-idf -C ${PahoTest_SOURCE_DIR} CLION_EXE_DIR=${PROJECT_BINARY_DIR})
add_custom_target(PahoTest_Flash COMMAND make flash IDF_PATH=/home/yesiot/workshop_esp32/esp-idf -C ${PahoTest_SOURCE_DIR} CLION_EXE_DIR=${PROJECT_BINARY_DIR})