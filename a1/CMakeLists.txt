cmake_minimum_required(VERSION 2.8.4)
project(a1)

add_executable(dolist list_driver.c list.c)
add_executable(dosync list_driver.c list_sync.c)
add_executable(handoff list_driver.c list_handoff.c)
