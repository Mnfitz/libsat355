# This is the CMakeLists for the app355 project (the main executable)
# app355 is meant to test various satellite tracking techniques

# Finds app355's cpp files to be used in this build
file(GLOB APP_FILES *.cpp)
# Set any external #defines (-D MYDEFINE) for app355
set(APP355_DEFINES) #Empty for now, but can be used to define things like _DEBUG or NDEBUG

# compile flags: -Wall "enable all compiler warnings"
# CMAKE_CXX_FLAGS is a preexisting magic supervariable that tells the compiler what compile options it should use
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
# c++ language version level: c++17
set(CMAKE_CXX_STANDARD 17)

# Define an executable called app355 using the cpp files found above
add_executable(app355 ${APP_FILES})
# Indicate the location to find #include (-I dir) files when compiling source
target_include_directories(app355 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# Applying any additional compiler options beyond what is specified in CMAKE_CXX_FLAGS
target_compile_definitions(app355 PRIVATE ${APP355_DEFINES})

# Tell CMake app355 executable requires the libsat355 library to link against
target_link_libraries(app355 PRIVATE libsat355)
# Tell cmake how to place the output executable in a tidy place a client can find it
install(TARGETS app355 DESTINATION build)

# Create a customized structure to hold header files when viewed through the IDE
# Header files will appear in an IDE folder called "Header Files"
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
