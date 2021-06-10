cmake_minimum_required(VERSION 3.10)

project(essence_fuzzer)

set(CMAKE_C_STANDARD 99)

include_directories(. ../src ../src/iota)

add_definitions(-DFUZZING)
    
add_executable(essence_fuzzer fuzztest.c ../src/iota/essence.c)
add_executable(essence_fuzzer_coverage fuzztest.c ../src/iota/essence.c)

target_compile_options(essence_fuzzer PRIVATE -fsanitize=fuzzer,address -g -ggdb2 -O1)
target_compile_options(essence_fuzzer_coverage PRIVATE -fsanitize=fuzzer,address -g -ggdb2 -O1 -fprofile-instr-generate -fcoverage-mapping)

target_link_options(essence_fuzzer PRIVATE -fsanitize=fuzzer,address)
target_link_options(essence_fuzzer_coverage PRIVATE -fsanitize=fuzzer,address)