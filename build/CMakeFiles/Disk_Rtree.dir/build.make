# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.31

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = D:\env\cmake\bin\cmake.exe

# The command to remove a file.
RM = D:\env\cmake\bin\cmake.exe -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = D:\project\MR_on_RTree

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = D:\project\MR_on_RTree\build

# Include any dependencies generated for this target.
include CMakeFiles/Disk_Rtree.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/Disk_Rtree.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/Disk_Rtree.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Disk_Rtree.dir/flags.make

CMakeFiles/Disk_Rtree.dir/codegen:
.PHONY : CMakeFiles/Disk_Rtree.dir/codegen

CMakeFiles/Disk_Rtree.dir/DiskRTreeTest.cpp.obj: CMakeFiles/Disk_Rtree.dir/flags.make
CMakeFiles/Disk_Rtree.dir/DiskRTreeTest.cpp.obj: D:/project/MR_on_RTree/DiskRTreeTest.cpp
CMakeFiles/Disk_Rtree.dir/DiskRTreeTest.cpp.obj: CMakeFiles/Disk_Rtree.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=D:\project\MR_on_RTree\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Disk_Rtree.dir/DiskRTreeTest.cpp.obj"
	D:\env\msys2\ucrt64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Disk_Rtree.dir/DiskRTreeTest.cpp.obj -MF CMakeFiles\Disk_Rtree.dir\DiskRTreeTest.cpp.obj.d -o CMakeFiles\Disk_Rtree.dir\DiskRTreeTest.cpp.obj -c D:\project\MR_on_RTree\DiskRTreeTest.cpp

CMakeFiles/Disk_Rtree.dir/DiskRTreeTest.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/Disk_Rtree.dir/DiskRTreeTest.cpp.i"
	D:\env\msys2\ucrt64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\project\MR_on_RTree\DiskRTreeTest.cpp > CMakeFiles\Disk_Rtree.dir\DiskRTreeTest.cpp.i

CMakeFiles/Disk_Rtree.dir/DiskRTreeTest.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/Disk_Rtree.dir/DiskRTreeTest.cpp.s"
	D:\env\msys2\ucrt64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\project\MR_on_RTree\DiskRTreeTest.cpp -o CMakeFiles\Disk_Rtree.dir\DiskRTreeTest.cpp.s

# Object files for target Disk_Rtree
Disk_Rtree_OBJECTS = \
"CMakeFiles/Disk_Rtree.dir/DiskRTreeTest.cpp.obj"

# External object files for target Disk_Rtree
Disk_Rtree_EXTERNAL_OBJECTS =

Disk_Rtree.exe: CMakeFiles/Disk_Rtree.dir/DiskRTreeTest.cpp.obj
Disk_Rtree.exe: CMakeFiles/Disk_Rtree.dir/build.make
Disk_Rtree.exe: CMakeFiles/Disk_Rtree.dir/linkLibs.rsp
Disk_Rtree.exe: CMakeFiles/Disk_Rtree.dir/objects1.rsp
Disk_Rtree.exe: CMakeFiles/Disk_Rtree.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=D:\project\MR_on_RTree\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Disk_Rtree.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\Disk_Rtree.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Disk_Rtree.dir/build: Disk_Rtree.exe
.PHONY : CMakeFiles/Disk_Rtree.dir/build

CMakeFiles/Disk_Rtree.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\Disk_Rtree.dir\cmake_clean.cmake
.PHONY : CMakeFiles/Disk_Rtree.dir/clean

CMakeFiles/Disk_Rtree.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" D:\project\MR_on_RTree D:\project\MR_on_RTree D:\project\MR_on_RTree\build D:\project\MR_on_RTree\build D:\project\MR_on_RTree\build\CMakeFiles\Disk_Rtree.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/Disk_Rtree.dir/depend

