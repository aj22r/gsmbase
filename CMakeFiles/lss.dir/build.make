# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.17

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

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "C:\Users\Artur\Desktop\Desktop2\vscode mcu\vscode_samd10c14\gsmbase"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "C:\Users\Artur\Desktop\Desktop2\vscode mcu\vscode_samd10c14\gsmbase"

# Utility rule file for lss.

# Include the progress variables for this target.
include CMakeFiles/lss.dir/progress.make

CMakeFiles/lss:
	arm-none-eabi-objdump -x -S build/gsmbase.elf > build/gsmbase.lss

lss: CMakeFiles/lss
lss: CMakeFiles/lss.dir/build.make

.PHONY : lss

# Rule to build all files generated by this target.
CMakeFiles/lss.dir/build: lss

.PHONY : CMakeFiles/lss.dir/build

CMakeFiles/lss.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\lss.dir\cmake_clean.cmake
.PHONY : CMakeFiles/lss.dir/clean

CMakeFiles/lss.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" "C:\Users\Artur\Desktop\Desktop2\vscode mcu\vscode_samd10c14\gsmbase" "C:\Users\Artur\Desktop\Desktop2\vscode mcu\vscode_samd10c14\gsmbase" "C:\Users\Artur\Desktop\Desktop2\vscode mcu\vscode_samd10c14\gsmbase" "C:\Users\Artur\Desktop\Desktop2\vscode mcu\vscode_samd10c14\gsmbase" "C:\Users\Artur\Desktop\Desktop2\vscode mcu\vscode_samd10c14\gsmbase\CMakeFiles\lss.dir\DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/lss.dir/depend

