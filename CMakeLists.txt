# require at least cmake v3.0.0, so we can use CTest
cmake_minimum_required(VERSION 3.13)
enable_testing()
# compile flags: -Wall "enable all compiler warnings"
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall /wd4514 /wd4668 /wd4062 /wd5267 /wd4061 /wd5264 /wd4820 /wd5038 /wd5045 /wd4577")
# c++ language version level: c++11
set(CMAKE_CXX_STANDARD 11)
# cmake project name: "libsat"
project(libsat VERSION 0.1.0)

# add CTest capability
include(CTest)

#Tricky: you determine whether you're build for x64(64bit) or x86(32bits) by examining the sizeof(void*) pointer you're compiling for.
math(EXPR platform_bits "${CMAKE_SIZEOF_VOID_P} * 8")
set(platform_dir ${CMAKE_SYSTEM_NAME}${platform_bits}-${CMAKE_CXX_COMPILER_ID}${CMAKE_CXX_COMPILER_VERSION})

foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)

    foreach(var CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config} CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config} CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config})
        #lib\win10-x86
        set(${var} "${CMAKE_BINARY_DIR}/../lib/win-x64")
        string(TOLOWER "${${var}}" ${var})
    endforeach()

endforeach()

MESSAGE (STATUS "${CMAKE_BINARY_DIR}/../lib/win-x64: " ${CMAKE_BINARY_DIR}/../lib/win-x64)

#include_directories(cppOrbitTools/orbitTools/core)
#include_directories(cppOrbitTools)
#file(GLOB SRC_FILES src/*.cpp cppOrbitTools/orbitTools/core/*.cpp cppOrbitTools/orbitTools/orbit/*.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

#add_library(libsat355 SHARED ${SRC_FILES})
#target_include_directories(libsat355 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#install(TARGETS libsat355 DESTINATION build)

find_program(GCOV_PATH gcov)
if(NOT GCOV_PATH)
  message(FATAL_ERROR "Code coverage analysis requires gcov!")
endif()


set_property(GLOBAL PROPERTY USE_FOLDERS ON)


add_subdirectory(src build)


add_subdirectory(tests)



## cmake compile
# cmake -G "Visual Studio 17 2022" -S . -B ./build

## cmake clean
# cmake --build ./build/ --target clean



#cmake_minimum_required(VERSION 3.13)
#
#project(gtest-demo LANGUAGES CXX)
#
## set the C++ standard
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)
#
## place binaries and libraries according to GNU standards
#include(GNUInstallDirs)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
#
## we cannot analyse results without gcov
#find_program(GCOV_PATH gcov)
#if(NOT GCOV_PATH)
#  message(FATAL_ERROR "Code coverage analysis requires gcov!")
#endif()
#
#add_subdirectory(src)
#
#enable_testing()

#add_subdirectory(test)
