get_filename_component(CURRENT_SOURCE_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(APP_NAME ${CURRENT_SOURCE_DIR_NAME}) # 工程名, 默认文件夹名

file(GLOB_RECURSE SOURCE_FILES 
    *.h
    *.cpp
    *.cc
    *.c
)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${LUA_INCLUDE_DIRS})

add_executable(${APP_NAME} ${SOURCE_FILES})
target_link_libraries(${APP_NAME} ${PHSYXSDK_LIBRARIES})
target_link_libraries(${APP_NAME} lua system)

if(APPLE)
    target_link_libraries(${APP_NAME} "-framework CoreFoundation")
endif(APPLE)

set_property(TARGET ${APP_NAME} PROPERTY FOLDER ${PROJ_NAME})