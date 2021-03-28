cmake_minimum_required(VERSION 3.12)
project(eth_booster VERSION 0.1.0 LANGUAGES CXX)

set(BUILD_SHARED_LIBS false)

if(WIN32)
    set(_PS "win")
elseif(UNIX)
    set(_PS "linux")
endif(WIN32)


message(STATUS ${MY_INCLUDES})

if(WIN32)
    add_library(eth
        win/iocp.cpp
        utils.cpp
        poolmgr.cpp
    )
    set(ADD_LIBS ws2_32)
elseif(UNIX)
    add_library(eth
        utils.cpp
        linux/epoll.cpp
    )
    set(ADD_LIBS pthread rt)
endif(WIN32)

target_compile_features(eth PRIVATE cxx_std_14)
#target_compile_options(eth PRIVATE -Wall -Wextra -Wpedantic)
#target_compile_definitions(eth PRIVATE DEFINES)
target_include_directories(eth PUBLIC . ${_PS})

add_executable(test_timestamp tests/test_timestamp.cpp)
target_include_directories(
    test_timestamp PUBLIC
    ${eth_booster_SOURCE_DIR}
    ${eth_booster_SOURCE_DIR}/${_PS}
)
target_link_libraries(test_timestamp eth)

#if (WIN32)
add_executable(test_basic_socket tests/test_basic_socket.cpp)
target_include_directories(
    test_basic_socket PUBLIC
    ${eth_booster_SOURCE_DIR}
    ${eth_booster_SOURCE_DIR}/${_PS}
)
target_link_libraries(test_basic_socket eth ${ADD_LIBS})
#endif(WIN32)

add_executable(test_pool tests/test_pool.cpp)
target_include_directories(
    test_pool PUBLIC
    ${eth_booster_SOURCE_DIR}
    ${eth_booster_SOURCE_DIR}/${_PS}
)

# ${CMAKE_CURRENT_SOURCE_DIR}
