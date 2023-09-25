get_filename_component(CURRENT_SOURCE_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(LIB_NAME ${CURRENT_SOURCE_DIR_NAME}) 

include_directories(${LUA_INCLUDE_DIRS})

include_directories(${OpenCV_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCE_FILES 
    *.h
    *.cpp
    *.cc
    *.c
)
add_library(${LIB_NAME} STATIC ${SOURCE_FILES})
target_link_libraries(${LIB_NAME} lua)
target_link_libraries(${LIB_NAME} ${OpenCV_LIBRARIES})

set_property(TARGET ${LIB_NAME} PROPERTY FOLDER ${PROJ_NAME})