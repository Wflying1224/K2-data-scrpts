# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/local/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC

# Utility rule file for sparsetest.

# Include the progress variables for this target.
include selfTest/quoc/CMakeFiles/sparsetest.dir/progress.make

selfTest/quoc/CMakeFiles/sparsetest: selfTest/quoc/sparsetest

sparsetest: selfTest/quoc/CMakeFiles/sparsetest
sparsetest: selfTest/quoc/CMakeFiles/sparsetest.dir/build.make
.PHONY : sparsetest

# Rule to build all files generated by this target.
selfTest/quoc/CMakeFiles/sparsetest.dir/build: sparsetest
.PHONY : selfTest/quoc/CMakeFiles/sparsetest.dir/build

selfTest/quoc/CMakeFiles/sparsetest.dir/clean:
	cd /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC/selfTest/quoc && $(CMAKE_COMMAND) -P CMakeFiles/sparsetest.dir/cmake_clean.cmake
.PHONY : selfTest/quoc/CMakeFiles/sparsetest.dir/clean

selfTest/quoc/CMakeFiles/sparsetest.dir/depend:
	cd /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh/selfTest/quoc /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC/selfTest/quoc /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC/selfTest/quoc/CMakeFiles/sparsetest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : selfTest/quoc/CMakeFiles/sparsetest.dir/depend

