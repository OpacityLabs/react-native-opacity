cmake_minimum_required(VERSION 3.4.1)
project(Opacity)

set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 20)

find_package(fbjni REQUIRED CONFIG)

add_library(react-native-opacity            STATIC
            ../cpp/NativeOpacityTurboModule.cpp
            cpp/OpacityCore.cpp
)

target_include_directories(react-native-opacity
        PUBLIC
            ../cpp
)

target_include_directories(react-native-opacity PUBLIC ../include)

set(OPACITY_CORE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/src/jniLibs/${ANDROID_ABI}/libopacity_core.a)
add_library(opacity_core STATIC IMPORTED)
set_target_properties(opacity_core PROPERTIES IMPORTED_LOCATION ${OPACITY_CORE_LIB})



target_link_libraries(react-native-opacity
        fbjni::fbjni
        opacity_core
        jsi
        react_nativemodule_core
        react_codegen_RNOpacitySpec
        log)
