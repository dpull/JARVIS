get_filename_component(CURRENT_SOURCE_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(APP_NAME ${CURRENT_SOURCE_DIR_NAME})

file(GLOB_RECURSE SOURCE_FILES 
    *.h
    *.cpp
    *.cc
    *.c
)

file(GLOB_RECURSE INCLUDE_FILES 
    ../include/*.h
    ../include/*.hpp
)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${LUA_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})

add_executable(${APP_NAME} ${SOURCE_FILES} ${INCLUDE_FILES} ${LUA_SOURCE_FILES})
target_link_libraries(${APP_NAME} ${CURL_LIBRARIES})
target_link_libraries(${APP_NAME} lua windows)

if(OpenCV_FOUND)
    target_link_libraries(${APP_NAME} ${OpenCV_LIBRARIES} opencv)
endif()

if(APPLE)
    target_link_libraries(${APP_NAME} "-framework CoreFoundation")
endif(APPLE)

set_property(TARGET ${APP_NAME} PROPERTY FOLDER ${PROJ_NAME})