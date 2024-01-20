cmake_minimum_required(VERSION 3.15)

file(GLOB APP_FILES *.swift)
add_executable(app355-swift ${APP_FILES})
target_link_libraries(app355-swift PRIVATE libsat355)

#target_compile_options(app355-swift PRIVATE -cxx-interoperability-mode=default)

#target_compile_options(app355-swift PRIVATE "SHELL:-cxx-interoperability-mode=default")

install(TARGETS app355-swift DESTINATION lib)

## This is the CMakeLists for the app355 project (the main executable)
## app355 is meant to test various satellite tracking techniques
#
## Finds app355's cpp files to be used in this build
#file(GLOB APP_FILES *.swift)
## Set any external #defines (-D MYDEFINE) for app355
#set(APP355_DEFINES) #Empty for now, but can be used to define things like _DEBUG or NDEBUG
#
#set(CMAKE_Swift_FLAGS "-sdk /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")
## Define an executable called app355 using the cpp files found above
#add_executable(app355 ${APP_FILES})
## Indicate the location to find #include (-I dir) files when compiling source
#target_include_directories(app355 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
## Applying any additional compiler options beyond what is specified in CMAKE_CXX_FLAGS
#target_compile_definitions(app355 PRIVATE ${APP355_DEFINES})
#
## Tell CMake app355 executable requires the libsat355 library to link against
#set_target_properties(app355 PROPERTIES LINKER_LANGUAGE Swift)
#
##set(CMAKE_Swift_LINK_EXECUTABLE "${CMAKE_Swift_COMPILER}")
#if(NOT CMAKE_Swift_LINK_EXECUTABLE)
#   if(CMAKE_Swift_COMPILER_TARGET)
#    set(CMAKE_Swift_LINK_EXECUTABLE "${CMAKE_Swift_COMPILER} -target <CMAKE_Swift_COMPILER_TARGET> -output-file-map <SWIFT_OUTPUT_FILE_MAP> -incremental -num-threads ${CMAKE_Swift_NUM_THREADS} -emit-executable -o <TARGET> -emit-module -emit-module-path <SWIFT_MODULE> -emit-dependencies <DEFINES> <FLAGS> <INCLUDES> <SWIFT_SOURCES> <LINK_FLAGS> <LINK_LIBRARIES>")
#   else()
#    set(CMAKE_Swift_LINK_EXECUTABLE "${CMAKE_Swift_COMPILER} -output-file-map <SWIFT_OUTPUT_FILE_MAP> -incremental -num-threads ${CMAKE_Swift_NUM_THREADS} -emit-executable -o <TARGET> -emit-module -emit-module-path <SWIFT_MODULE> -emit-dependencies <DEFINES> <FLAGS> <INCLUDES> <SWIFT_SOURCES> <LINK_FLAGS> <LINK_LIBRARIES>")
#   endif()
# endif()
#
# 
## Create a customized structure to hold header files when viewed through the IDE
## Header files will appear in an IDE folder called "Header Files"
#source_group(
#  TREE "${PROJECT_SOURCE_DIR}/include"
#  PREFIX "Header Files"
#  FILES ${HEADER_LIST})
#