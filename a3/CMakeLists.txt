cmake_minimum_required(VERSION 2.8.4)
project(a3)

add_library(ext2 ext2.c ext2.h)

add_executable(ext2_cp
               ext2
               ext2_cp.c)

add_executable(ext2_mkdir
               ext2
               ext2_mkdir.c)

add_executable(ext2_ln
               ext2_ln.c)

add_executable(ext2_rm
               ext2_rm.c)
