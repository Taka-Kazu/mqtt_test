cmake_minimum_required(VERSION 2.8)
project(mqtt_test)

set(CMAKE_CXX_FLAGS "-O2 -std=c++11 -Wall")

# for json
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)

add_executable(mqtt_test src/mqtt_test.cpp)
target_link_libraries(mqtt_test mosquitto nlohmann_json::nlohmann_json)
