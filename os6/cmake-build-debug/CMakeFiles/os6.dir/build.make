# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.14

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /tmp/tmp.V9rPe33f2F

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /tmp/tmp.V9rPe33f2F/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/os6.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/os6.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/os6.dir/flags.make

CMakeFiles/os6.dir/main.c.o: CMakeFiles/os6.dir/flags.make
CMakeFiles/os6.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.V9rPe33f2F/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/os6.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/os6.dir/main.c.o   -c /tmp/tmp.V9rPe33f2F/main.c

CMakeFiles/os6.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/os6.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /tmp/tmp.V9rPe33f2F/main.c > CMakeFiles/os6.dir/main.c.i

CMakeFiles/os6.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/os6.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /tmp/tmp.V9rPe33f2F/main.c -o CMakeFiles/os6.dir/main.c.s

# Object files for target os6
os6_OBJECTS = \
"CMakeFiles/os6.dir/main.c.o"

# External object files for target os6
os6_EXTERNAL_OBJECTS =

os6: CMakeFiles/os6.dir/main.c.o
os6: CMakeFiles/os6.dir/build.make
os6: CMakeFiles/os6.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/tmp/tmp.V9rPe33f2F/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable os6"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/os6.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/os6.dir/build: os6

.PHONY : CMakeFiles/os6.dir/build

CMakeFiles/os6.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/os6.dir/cmake_clean.cmake
.PHONY : CMakeFiles/os6.dir/clean

CMakeFiles/os6.dir/depend:
	cd /tmp/tmp.V9rPe33f2F/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/tmp.V9rPe33f2F /tmp/tmp.V9rPe33f2F /tmp/tmp.V9rPe33f2F/cmake-build-debug /tmp/tmp.V9rPe33f2F/cmake-build-debug /tmp/tmp.V9rPe33f2F/cmake-build-debug/CMakeFiles/os6.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/os6.dir/depend
