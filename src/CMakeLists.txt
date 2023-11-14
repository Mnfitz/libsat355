include_directories(../cppOrbitTools/orbitTools/core)
include_directories(../cppOrbitTools)
file(GLOB SRC_FILES *.cpp ../cppOrbitTools/orbitTools/core/*.cpp ../cppOrbitTools/orbitTools/orbit/*.cpp)
set(LIBSAT355_DEFINES DLL_EXPORTS) #Note EXPORTS, plural, meaning use __declspec(dllexport) instead of __declspec(dllimport)
enable_testing()
add_library(libsat355 SHARED ${SRC_FILES})
target_include_directories(libsat355 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(libsat355 PUBLIC ${LIBSAT355_DEFINES})
install(TARGETS libsat355 DESTINATION build)

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})


#add_library(example)
#
#target_sources(example
#  PRIVATE
#    example.cpp
#  PUBLIC
#    example.h
#  )
#
#target_include_directories(example
#  PUBLIC
#    ${CMAKE_CURRENT_LIST_DIR}
#  )
#
## we use this to get code coverage
## flags are only valid with the GNU compiler and on Linux
#if(CMAKE_CXX_COMPILER_ID MATCHES GNU AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
#  target_compile_options(example
#    PUBLIC
#      "--coverage"
#    )
#  target_link_options(example
#    INTERFACE
#      "--coverage"
#    )
#endif()