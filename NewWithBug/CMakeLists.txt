cmake_minimum_required(VERSION 3.9)
project(raft C)

set(CMAKE_C_STANDARD 99)

add_library(raft raft.c raft.h election.c election.h logupdatecommands.c logupdatecommands.h main.c)