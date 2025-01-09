cmake_minimum_required(VERSION 2.5)
project(judger C)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output)

set(CMAKE_C_FLAGS "-g -Wall -Werror -O3 -std=c99 -static -pie -fPIC")

# make judger lib
file(GLOB SOURCE "src/*.c" "src/rules/*.c")
add_executable(libjudger.a ${SOURCE})
target_link_libraries(libjudger.a pthread seccomp)


install(FILES output/libjudger.a
    PERMISSIONS OWNER_EXECUTE OWNER_READ
    DESTINATION /usr/lib/judger)
