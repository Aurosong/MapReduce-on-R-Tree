cmake_minimum_required(VERSION 3.10)
project(MR_on_Rtree VERSION 0.1.0 LANGUAGES CXX)

# include_directories(Rtree)
# include_directories(Rtree_on_disk)
# include_directories(MapReduce)
# include_directories(Test)

add_executable(Memory_Rtree RtreeTest.cpp)
# add_executable(Disk_Rtree DiskRTreeTest.cpp)
# add_executable(test main.cpp)