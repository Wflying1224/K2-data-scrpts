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

# Include any dependencies generated for this target.
include selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/depend.make

# Include the progress variables for this target.
include selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/progress.make

# Include the compile flags for this target's objects.
include selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/flags.make

selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o: selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/flags.make
selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o: /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh/selfTest/quoc/selfTest.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o"
	cd /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC/selfTest/quoc && bash /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh/util/cmakeParseError.sh /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o -c /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh/selfTest/quoc/selfTest.cpp

selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.i"
	cd /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC/selfTest/quoc && bash /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh/util/cmakeParseError.sh /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh/selfTest/quoc/selfTest.cpp > CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.i

selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.s"
	cd /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC/selfTest/quoc && bash /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh/util/cmakeParseError.sh /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh/selfTest/quoc/selfTest.cpp -o CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.s

selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o.requires:
.PHONY : selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o.requires

selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o.provides: selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o.requires
	$(MAKE) -f selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/build.make selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o.provides.build
.PHONY : selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o.provides

selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o.provides.build: selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o

# Object files for target selfTest_quoc_selfTest
selfTest_quoc_selfTest_OBJECTS = \
"CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o"

# External object files for target selfTest_quoc_selfTest
selfTest_quoc_selfTest_EXTERNAL_OBJECTS =

selfTest/quoc/selfTest: selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o
selfTest/quoc/selfTest: selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/build.make
selfTest/quoc/selfTest: /usr/lib64/libbz2.so
selfTest/quoc/selfTest: /usr/lib64/libpng.so
selfTest/quoc/selfTest: /usr/lib64/libz.so
selfTest/quoc/selfTest: libquocmesh.so
selfTest/quoc/selfTest: /usr/lib64/libbz2.so
selfTest/quoc/selfTest: /usr/lib64/libpng.so
selfTest/quoc/selfTest: /usr/lib64/libz.so
selfTest/quoc/selfTest: selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable selfTest"
	cd /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC/selfTest/quoc && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/selfTest_quoc_selfTest.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/build: selfTest/quoc/selfTest
.PHONY : selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/build

selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/requires: selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/selfTest.cpp.o.requires
.PHONY : selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/requires

selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/clean:
	cd /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC/selfTest/quoc && $(CMAKE_COMMAND) -P CMakeFiles/selfTest_quoc_selfTest.dir/cmake_clean.cmake
.PHONY : selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/clean

selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/depend:
	cd /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocmesh/selfTest/quoc /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC/selfTest/quoc /home/peizhang/Hour_00/K2-data-scripts/quocmesh-20140902-1202/quocGCC/selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : selfTest/quoc/CMakeFiles/selfTest_quoc_selfTest.dir/depend

