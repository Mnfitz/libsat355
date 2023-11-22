
# Allow testing
enable_testing()
# Voodoo to fetch googletest from github
include(FetchContent)
FetchContent_Declare(gtest
  QUIET
  URL https://github.com/google/googletest/archive/release-1.10.0.tar.gz
)
# configure build of googletest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gtest)

# Define an executable called unit_test using test1.cpp
add_executable(unit_tests test1.cpp)

# Compile options to silence warnings
if(WIN32)
  target_compile_options(unit_tests PRIVATE "/EHsc")
elseif(APPLE)
  target_compile_options(unit_tests PRIVATE )
elseif(IOS)
  target_compile_options(unit_tests PRIVATE )
endif()

#add_test(NAME MyTest COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR} ctest -R MyTest)
add_test(NAME unit_tests
    COMMAND ${CMAKE_BINARY_DIR}/../lib/win-x64:unit_tests
    -E chdir ${CMAKE_BINARY_DIR}/../lib/win-x64 ctest -R unit_tests
    )

# Link the unit_test to the library libsat355 and gtest_main
target_link_libraries(unit_tests
  PRIVATE
    libsat355
    gtest_main
  )
# automatic discovery of unit tests
include(GoogleTest)
gtest_discover_tests(unit_tests
  PROPERTIES
    LABELS "unit"
  DISCOVERY_TIMEOUT  # how long to wait (in seconds) before crashing
    240
  )
