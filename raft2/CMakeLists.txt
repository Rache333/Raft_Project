cmake_minimum_required(VERSION 3.9)
project(raft2 C)

set(CMAKE_C_STANDARD 99)

add_library(raft2 election.c election.h log_replication.c log_replication.h logupdatecommands.c logupdatecommands.h multicast_handling.c multicast_handling.h raft.c raft.h)