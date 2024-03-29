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
add_executable(app355-cpp ${APP_FILES})
# Indicate the location to find #include (-I dir) files when compiling source
target_include_directories(app355-cpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# Applying any additional compiler options beyond what is specified in CMAKE_CXX_FLAGS
target_compile_definitions(app355-cpp PRIVATE ${APP355_DEFINES})

# Tell CMake app355 executable requires the libsat355 library to link against
target_link_libraries(app355-cpp PRIVATE libsat355)

if(WIN32)
  install(TARGETS app355-cpp DESTINATION lib/win-x64)
  set_target_properties(app355-cpp PROPERTIES PDB_NAME app355-cpp)
  install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib/win-x64/app355-cpp.pdb DESTINATION lib/win-x64)
endif()

# Create a customized structure to hold header files when viewed through the IDE
# Header files will appear in an IDE folder called "Header Files"
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
