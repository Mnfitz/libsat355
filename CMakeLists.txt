# require at least cmake v3.0.0, so we can use CTest
cmake_minimum_required(VERSION 3.13)

# compile flags: -Wall "enable all compiler warnings"
# CMAKE_CXX_FLAGS is a preexisting magic supervariable that tells the compiler what compile options it should use
set(CMAKE_SYSTEM_NAME "iOS")
set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS14.0.sdk")

if(WIN32)
    # Windows-specific compile settings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall /wd4514 /wd4668 /wd4062 /wd5267 /wd4061 /wd5264 /wd4820 /wd5038 /wd5045 /wd4577")
elseif(APPLE)
    # Mac-specific compile settings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -Wextra -Wpedantic")
elseif(IOS)
    # iOS-specific compile settings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -Wextra -Wpedantic -miphoneos-version-min=8.0")
endif()


# c++ language version level: c++11
set(CMAKE_CXX_STANDARD 11)
# cmake project name: "libsat"
project(libsat VERSION 0.1.0)

# add CTest capability
include(CTest)

#Tricky: you determine whether you're build for x64(64bit) or x86(32bits) by examining the sizeof(void*) pointer you're compiling for.
math(EXPR platform_bits "${CMAKE_SIZEOF_VOID_P} * 8")
set(platform_dir ${CMAKE_SYSTEM_NAME}${platform_bits}-${CMAKE_CXX_COMPILER_ID}${CMAKE_CXX_COMPILER_VERSION})

# Set the output directory for each build configuration
foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)

    foreach(var CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config} CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config} CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config})
        #lib\win10-x86
        set(${var} "${CMAKE_BINARY_DIR}/../lib/win-x64")
        string(TOLOWER "${${var}}" ${var})
    endforeach()

endforeach()

# Set the CPACK variables
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

find_program(GCOV_PATH gcov)
if(NOT GCOV_PATH)
  message(FATAL_ERROR "Code coverage analysis requires gcov!")
endif()

# Organizes project into folders to reduce clutter
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Include, build, and read Makefiles from all subdirectories
add_subdirectory(src build)
add_subdirectory(app)
# enable testing
enable_testing()
add_subdirectory(tests)

## cmake compile
# cmake -G "Visual Studio 17 2022" -S . -B ./build

## cmake clean
# cmake --build ./build/ --target clean