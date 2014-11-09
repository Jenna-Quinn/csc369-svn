cmake_minimum_required(VERSION 2.8.4)
project(a2)

add_executable(sim
               sim.c
               pagetable.h
               pagetable.c
               avl.h
               avl.c
               rand.c
               clock.c
               lru.c
               fifo.c
               opt.c)

add_executable(simpleloop
               simpleloop.c)

add_executable(matmul
               matmul.c
               timer.h)

add_executable(blocked
               blocked.c
               timer.h)
