# require at least cmake v3.0.0, so we can use CTest
cmake_minimum_required(VERSION 3.13)

# compile flags: -Wall "enable all compiler warnings"
# CMAKE_CXX_FLAGS is a preexisting magic supervariable that tells the compiler what compile options it should use

#set(CMAKE_SYSTEM_NAME "APPLE")

if(WIN32)
    # Windows-specific compile settings
    set(LIBSAT355_LANGUAGES CXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall /wd4514 /wd4668 /wd4062 /wd5267 /wd4061 /wd5264 /wd4820 /wd5038 /wd5045 /wd4577")
elseif(APPLE)
    # Settings common to both Mac and iOS
    set(LIBSAT355_LANGUAGES Swift CXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall -Wextra -Wpedantic -Wno-nonportable-include-path -Wno-unused-const-variable")
    set(SWIFT_FLAGS "${SWIFT_FLAGS}  -cxx-interoperability-mode=default")
    if(IOS)
        # ios-specific compile settings
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -miphoneos-version-min=8.0")
        set(SWIFT_FLAGS "${SWIFT_FLAGS} ")
        set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS16.2.sdk")
        set(CMAKE_OSX_ARCHITECTURES "arm64")
    else()
        # Mac-specific compile settings
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
        set(SWIFT_FLAGS "${SWIFT_FLAGS} ")
        set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")
    endif()
endif()

# c++ language version level: c++17
set(CMAKE_CXX_STANDARD 17)
# cmake project name: "libsat"

project(libsat VERSION 0.1.0 LANGUAGES ${LIBSAT355_LANGUAGES})

# add CTest capability
include(CTest)

#Tricky: you determine whether you're build for x64(64bit) or x86(32bits) by examining the sizeof(void*) pointer you're compiling for.
math(EXPR platform_bits "${CMAKE_SIZEOF_VOID_P} * 8")
set(platform_dir ${CMAKE_SYSTEM_NAME}${platform_bits}-${CMAKE_CXX_COMPILER_ID}${CMAKE_CXX_COMPILER_VERSION})

# Set the output directory for each build configuration

foreach(config _DEBUG _RELEASE _RELWITHDEBINFO _MINSIZEREL "")

    foreach(var CMAKE_ARCHIVE_OUTPUT_DIRECTORY${config} CMAKE_LIBRARY_OUTPUT_DIRECTORY${config} CMAKE_RUNTIME_OUTPUT_DIRECTORY${config})
        #lib\win10-x86
        if(WIN32)
            set(${var} "${CMAKE_BINARY_DIR}/../lib/win-x64")
        elseif(APPLE)
            if(IOS)
                set(${var} "${CMAKE_BINARY_DIR}/../lib/ios")
            else()
                set(${var} "${CMAKE_BINARY_DIR}/../lib/macos")
            endif()
        endif()
        string(TOLOWER "${${var}}" ${var})
        #MESSAGE(${var})
        #MESSAGE(${${var}})
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

# Adds /src to the header search path, so we can #include labsat355.h
include_directories(src)
# Invoke the build CMakeLists.txt instructions for libsat355
add_subdirectory(src build)

# Tell cmake how to place the output executable in a tidy place a client can find it
if(NOT APPLE)
    # Invoke the CMakeLists.txt build instructions for app355-cpp
    add_subdirectory(app-cpp)
    install(TARGETS app355-cpp DESTINATION out/app-cpp)
else()
    # Invoke the CMakeLists.txt build instructions for app-swift 
    add_subdirectory(app-swift)
    install(TARGETS app355-swift DESTINATION out/app-swift)
endif()

# Tell cmake how to place the output executable in a tidy place a client can find it
if(NOT IOS)
# enable testing
    enable_testing()
    add_subdirectory(tests)
    install(TARGETS unit_tests DESTINATION out/tests)
endif()