# require at least cmake v3.0.0, so we can use CTest
cmake_minimum_required(VERSION 2.8...3.13)

# compile flags: -Wall "enable all compiler warnings"
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# c++ language version level: c++11
set(CMAKE_CXX_STANDARD 11)
# cmake project name: "project1"
project(libsat355 VERSION 0.1.0)

# add CTest capability to "cse340"
include(CTest)
enable_testing()


if( CMAKE_RUNTIME_OUTPUT_DIRECTORY )
	set( executableOutputPath ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/build )
elseif( EXECUTABLE_OUTPUT_PATH )
	set( executableOutputPath ${EXECUTABLE_OUTPUT_PATH}/build )
else()
	set( executableOutputPath ${CMAKE_CURRENT_BINARY_DIR}/build )
endif()

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY build)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY build)


include_directories(cppOrbitTools/orbitTools/core)
file(GLOB SRC_FILES helloworld.cpp dllmain.cpp cppOrbitTools/orbitTools/core/*.cpp)
add_library(libsat355 ${SRC_FILES})
#add_executable(a.out ${SRC_FILES})

# project1 produces an executable (app) named: "a.out"
# that takes as input 4 ".cc" source files...
#add_executable(a.out helloworld.cc cppOrbitTools\orbitTools\core\cEci.cpp cppOrbitTools\orbitTools\core\cJulian.cpp 
#cppOrbitTools\orbitTools\core\cVector.cpp cppOrbitTools\orbitTools\core\cEcef.cpp cppOrbitTools\orbitTools\core\cCoordGeodetic.cpp 
#cppOrbitTools\orbitTools\core\cOrbit.cpp cppOrbitTools\orbitTools\core\cSite.cpp cppOrbitTools\orbitTools\core\cJulian.cpp 
#cppOrbitTools\orbitTools\core\cVector.cpp cppOrbitTools\orbitTools\core\cEci.cpp cppOrbitTools\orbitTools\core\cEcef.cpp 
#cppOrbitTools\orbitTools\core\cCoordGeodetic.cpp cppOrbitTools\orbitTools\core\cOrbit.cpp cppOrbitTools\orbitTools\core\cSite.cpp 
#cppOrbitTools\orbitTools\core\cJulian.cpp cppOrbitTools\orbitTools\core\cVector.cpp cppOrbitTools\orbitTools\core\cEci.cpp 
#cppOrbitTools\orbitTools\core\cEcef.cpp cppOrbitTools\orbitTools\core\cCoordGeodetic.cpp cppOrbitTools\orbitTools\core\cOrbit.cpp 
#cppOrbitTools\orbitTools\core\cSite.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


# require at least cmake v3.0.0, so we can use CTest
#cmake_minimum_required(VERSION 3.0.0)

# compile flags: -Wall "enable all compiler warnings"
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# c++ language version level: c++11
#set(CMAKE_CXX_STANDARD 11)

# cmake project name: "project1"
#project(project1 VERSION 0.1.0)

# add CTest capability to "cse340"
#include(CTest)
#enable_testing()

# project1 produces an executable (app) named: "a.out"
# that takes as input 4 ".cc" source files...
#add_executable(a.out main.cc inputbuf.cc lexer.cc parser.cc symboltable.cc memorytable.cc proctable.cc)

#set(CPACK_PROJECT_NAME ${PROJECT_NAME})
#set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
#include(CPack)


# cmake -G "Visual Studio 17 2022"