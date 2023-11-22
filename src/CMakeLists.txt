# Directories to read from
include_directories(../cppOrbitTools/orbitTools/core)
include_directories(../cppOrbitTools)
#  Finds libsat355's cpp files to be used in this build
file(GLOB SRC_FILES libsat355.cpp ../cppOrbitTools/orbitTools/core/*.cpp ../cppOrbitTools/orbitTools/orbit/*.cpp)
# Set any external #defines (-D MYDEFINE) for libsat355
set(LIBSAT355_DEFINES DLL_EXPORTS) #Note EXPORTS, plural, meaning use __declspec(dllexport) instead of __declspec(dllimport)

# Define a library called libsat355 using the cpp files found above

if(WIN32)
  # Include windows specific dll entrypoint
  list(APPEND SRC_FILES dllmain.cpp)
  add_library(libsat355 SHARED ${SRC_FILES})
elseif(APPLE)
  add_library(libsat355 SHARED ${SRC_FILES})
elseif(IOS)
  add_library(libsat355 STATIC ${SRC_FILES})
endif()


#add_library(libsat355 SHARED ${SRC_FILES})
# Applying any additional compiler options beyond what is specified in CMAKE_CXX_FLAGS
target_compile_definitions(libsat355 PUBLIC ${LIBSAT355_DEFINES})

# Indicate the location to find #include (-I dir) files when compiling source
target_include_directories(libsat355 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(libsat355 PUBLIC ../cppOrbitTools)

# Tell cmake how to place the output executable in a tidy place a client can find it
install(TARGETS libsat355 DESTINATION build)

# Create a customized structure to hold header files when viewed through the IDE
# Header files will appear in an IDE folder called "Header Files"
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})