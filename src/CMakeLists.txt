include_directories(../cppOrbitTools/orbitTools/core)
include_directories(../cppOrbitTools)
file(GLOB SRC_FILES *.cpp ../cppOrbitTools/orbitTools/core/*.cpp ../cppOrbitTools/orbitTools/orbit/*.cpp)
set(LIBSAT355_DEFINES DLL_EXPORTS) #Note EXPORTS, plural, meaning use __declspec(dllexport) instead of __declspec(dllimport)

add_library(libsat355 SHARED ${SRC_FILES})
target_compile_definitions(libsat355 PUBLIC ${LIBSAT355_DEFINES})

target_include_directories(libsat355 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(libsat355 PUBLIC ../cppOrbitTools)

install(TARGETS libsat355 DESTINATION build)

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})