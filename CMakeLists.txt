# require at least cmake v3.0.0, so we can use CTest
cmake_minimum_required(VERSION 2.8...3.13)

# compile flags: -Wall "enable all compiler warnings"
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# c++ language version level: c++11
set(CMAKE_CXX_STANDARD 11)
# cmake project name: "project1"
project(libsat355 VERSION 0.1.0)

# add CTest capability
include(CTest)
enable_testing()


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

 ## Output directory for libraries in Debug configuration
 #set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG})
 ## Output directory for libraries in Release configuration
 #set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/${OUTPUT_REL})


 #if(CMAKE_BUILD_TYPE STREQUAL "Debug")
 #set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
 #set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
 #set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
#else()
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
# #set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
# #set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
#endif()


#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY build)

include_directories(cppOrbitTools/orbitTools/core)
file(GLOB SRC_FILES helloworld.cpp dllmain.cpp cppOrbitTools/orbitTools/core/*.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_library(libsat SHARED ${SRC_FILES})
target_include_directories(libsat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS libsat DESTINATION build)

# cmake -G "Visual Studio 17 2022"
# cmake -G "Visual Studio 17 2022" -S . -B ./build