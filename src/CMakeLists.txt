#  Finds libsat355's cpp files to be used in this build
file(GLOB SRC_FILES "libsat355.cpp" "../cppOrbitTools/orbitTools/core/*.cpp" "../cppOrbitTools/orbitTools/orbit/*.cpp")
# Set any external #defines (-D MYDEFINE) for libsat355
set(LIBSAT355_DEFINES DLL_EXPORTS) #Note -DDLL_EXPORTS, plural, meaning use __declspec(dllexport) instead of __declspec(dllimport)

# Define a library called libsat355 using the cpp files found above

if(WIN32)
  # Include windows specific dll entrypoint
  list(APPEND SRC_FILES dllmain.cpp)
  add_library(libsat355 SHARED ${SRC_FILES})
elseif(APPLE)
  if(IOS)
    add_library(libsat355 STATIC ${SRC_FILES})
  else()
    add_library(libsat355 SHARED ${SRC_FILES})
  endif()
endif()

  #set(PRECOMPILE_HEADERS "../cppOrbitTools/stdafx.h")
  #set(PRECOMPILE_BINARY "stdafx.h.pch")
  #execute_process(COMMAND ${CMAKE_CXX_COMPILER} -x c++-header ${PRECOMPILE_HEADERS} -o ${PRECOMPILE_BINARY})
  #target_precompile_headers(libsat355 REUSE_FROM ${PRECOMPILE_BINARY})
  #target_precompile_headers(libsat355 PRIVATE ${PRECOMPILE_HEADERS})


# Applying any additional compiler options beyond what is specified in CMAKE_CXX_FLAGS
target_compile_definitions(libsat355 PRIVATE ${LIBSAT355_DEFINES})

# Indicate the location to find #include (-I dir) files when compiling source
# TRICKY: We must place this include path first to override bogus headers in the zeptomoby code
target_include_directories(libsat355 BEFORE PRIVATE ../cppOrbitTools)
target_include_directories(libsat355 AFTER PRIVATE ../cppOrbitTools/overrides/core ../cppOrbitTools/overrides/orbit)
target_include_directories(libsat355 AFTER PRIVATE ../cppOrbitTools/orbitTools/core ../cppOrbitTools/orbitTools/orbit ${CMAKE_CURRENT_SOURCE_DIR})

# Tell cmake how to place the output executable in a tidy place a client can find it
install(TARGETS libsat355 DESTINATION lib)

if(WIN32)
  set_target_properties(libsat355 PROPERTIES PDB_NAME libsat355)
  install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib/libsat355.pdb DESTINATION lib)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/libsat355.h DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
elseif(APPLE)
  if(IOS)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/libsat355.h DESTINATION ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/module.modulemap DESTINATION ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
  else()
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/libsat355.h DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/module.modulemap DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  endif()
endif()

# Create a customized structure to hold header files when viewed through the IDE
# Header files will appear in an IDE folder called "Header Files"
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})