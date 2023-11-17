file(GLOB APP_FILES *.cpp)
#set(APP355_DEFINES ) #Empty for now, but can be used to define things like _DEBUG or NDEBUG

add_executable(app355 ${APP_FILES})
target_include_directories(app355 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(app355 PUBLIC ${APP355_DEFINES})

target_link_libraries(app355
  PRIVATE
    libsat355
  )
install(TARGETS app355 DESTINATION build)

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})


#add_library(mylib SHARED IMPORTED)
#set_target_properties(mylib PROPERTIES IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/res/mylib.so)
#target_link_libraries(MyExecutable mylib)
#
#
#add_library(MyLibrary ${LIB_SOURCES}) # Create your library
#add_executable(MyExecutable ${EXE_SOURCES}) # Create your executable
#target_link_libraries(MyExecutable MyLibrary) # Link your executable against your library

#target_link_libraries(app355
#  PRIVATE
#    libsat355
#  )