aux_source_directory(. SRC_LIST)
aux_source_directory(../net SRC_LIST1)

include_directories(. ../net ../pubutils)

add_executable(smallchat-server ${SRC_LIST} ${SRC_LIST1})

target_link_libraries(smallchat-server pthread)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)