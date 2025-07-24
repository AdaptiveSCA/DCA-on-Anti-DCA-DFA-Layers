CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(WBMatrix)

include_directories(include)

aux_source_directory(src/WBMatrix WBM_SRC)

add_library(WB_LIB ${WBM_SRC})

link_libraries(m)

add_executable(ENCA test/main.c)

target_link_libraries(ENCA WB_LIB)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -lm -O2")