cmake_minimum_required(VERSION 3.22.1)

project("OpacityCore")

add_library(${CMAKE_PROJECT_NAME} SHARED
    OpacityCore.cpp)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/../jni/include)

# Import the shared library but don't embed the absolute path
add_library(sdk SHARED IMPORTED)
set_target_properties(sdk PROPERTIES 
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libsdk.so
    IMPORTED_NO_SONAME ON)

target_link_libraries(${CMAKE_PROJECT_NAME}
    sdk
    android
    log)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    "-Wl,-z,max-page-size=16384")