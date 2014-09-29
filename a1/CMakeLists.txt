cmake_minimum_required(VERSION 2.8.4)
project(a1)

add_executable(test_malloc test_malloc.c mymemory.c)
add_executable(test_malloc_opt test_malloc.c mymemory_opt.c)
