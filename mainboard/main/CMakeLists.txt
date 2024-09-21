file(GLOB_RECURSE SRC_FILES "src/*.cpp" "src/*.c")

idf_component_register(SRCS ${SRC_FILES}
                    INCLUDE_DIRS "./include")
