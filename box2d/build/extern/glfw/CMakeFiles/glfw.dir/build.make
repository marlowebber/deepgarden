# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/marlo/Documents/deepsea/box2d

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/marlo/Documents/deepsea/box2d/build

# Include any dependencies generated for this target.
include extern/glfw/CMakeFiles/glfw.dir/depend.make

# Include the progress variables for this target.
include extern/glfw/CMakeFiles/glfw.dir/progress.make

# Include the compile flags for this target's objects.
include extern/glfw/CMakeFiles/glfw.dir/flags.make

extern/glfw/CMakeFiles/glfw.dir/src/context.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/context.c.o: ../extern/glfw/src/context.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/context.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/context.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/context.c

extern/glfw/CMakeFiles/glfw.dir/src/context.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/context.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/context.c > CMakeFiles/glfw.dir/src/context.c.i

extern/glfw/CMakeFiles/glfw.dir/src/context.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/context.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/context.c -o CMakeFiles/glfw.dir/src/context.c.s

extern/glfw/CMakeFiles/glfw.dir/src/init.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/init.c.o: ../extern/glfw/src/init.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/init.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/init.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/init.c

extern/glfw/CMakeFiles/glfw.dir/src/init.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/init.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/init.c > CMakeFiles/glfw.dir/src/init.c.i

extern/glfw/CMakeFiles/glfw.dir/src/init.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/init.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/init.c -o CMakeFiles/glfw.dir/src/init.c.s

extern/glfw/CMakeFiles/glfw.dir/src/input.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/input.c.o: ../extern/glfw/src/input.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/input.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/input.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/input.c

extern/glfw/CMakeFiles/glfw.dir/src/input.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/input.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/input.c > CMakeFiles/glfw.dir/src/input.c.i

extern/glfw/CMakeFiles/glfw.dir/src/input.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/input.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/input.c -o CMakeFiles/glfw.dir/src/input.c.s

extern/glfw/CMakeFiles/glfw.dir/src/monitor.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/monitor.c.o: ../extern/glfw/src/monitor.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/monitor.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/monitor.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/monitor.c

extern/glfw/CMakeFiles/glfw.dir/src/monitor.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/monitor.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/monitor.c > CMakeFiles/glfw.dir/src/monitor.c.i

extern/glfw/CMakeFiles/glfw.dir/src/monitor.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/monitor.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/monitor.c -o CMakeFiles/glfw.dir/src/monitor.c.s

extern/glfw/CMakeFiles/glfw.dir/src/vulkan.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/vulkan.c.o: ../extern/glfw/src/vulkan.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/vulkan.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/vulkan.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/vulkan.c

extern/glfw/CMakeFiles/glfw.dir/src/vulkan.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/vulkan.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/vulkan.c > CMakeFiles/glfw.dir/src/vulkan.c.i

extern/glfw/CMakeFiles/glfw.dir/src/vulkan.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/vulkan.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/vulkan.c -o CMakeFiles/glfw.dir/src/vulkan.c.s

extern/glfw/CMakeFiles/glfw.dir/src/window.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/window.c.o: ../extern/glfw/src/window.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/window.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/window.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/window.c

extern/glfw/CMakeFiles/glfw.dir/src/window.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/window.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/window.c > CMakeFiles/glfw.dir/src/window.c.i

extern/glfw/CMakeFiles/glfw.dir/src/window.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/window.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/window.c -o CMakeFiles/glfw.dir/src/window.c.s

extern/glfw/CMakeFiles/glfw.dir/src/x11_init.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/x11_init.c.o: ../extern/glfw/src/x11_init.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/x11_init.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/x11_init.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/x11_init.c

extern/glfw/CMakeFiles/glfw.dir/src/x11_init.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/x11_init.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/x11_init.c > CMakeFiles/glfw.dir/src/x11_init.c.i

extern/glfw/CMakeFiles/glfw.dir/src/x11_init.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/x11_init.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/x11_init.c -o CMakeFiles/glfw.dir/src/x11_init.c.s

extern/glfw/CMakeFiles/glfw.dir/src/x11_monitor.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/x11_monitor.c.o: ../extern/glfw/src/x11_monitor.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/x11_monitor.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/x11_monitor.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/x11_monitor.c

extern/glfw/CMakeFiles/glfw.dir/src/x11_monitor.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/x11_monitor.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/x11_monitor.c > CMakeFiles/glfw.dir/src/x11_monitor.c.i

extern/glfw/CMakeFiles/glfw.dir/src/x11_monitor.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/x11_monitor.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/x11_monitor.c -o CMakeFiles/glfw.dir/src/x11_monitor.c.s

extern/glfw/CMakeFiles/glfw.dir/src/x11_window.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/x11_window.c.o: ../extern/glfw/src/x11_window.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/x11_window.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/x11_window.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/x11_window.c

extern/glfw/CMakeFiles/glfw.dir/src/x11_window.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/x11_window.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/x11_window.c > CMakeFiles/glfw.dir/src/x11_window.c.i

extern/glfw/CMakeFiles/glfw.dir/src/x11_window.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/x11_window.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/x11_window.c -o CMakeFiles/glfw.dir/src/x11_window.c.s

extern/glfw/CMakeFiles/glfw.dir/src/xkb_unicode.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/xkb_unicode.c.o: ../extern/glfw/src/xkb_unicode.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/xkb_unicode.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/xkb_unicode.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/xkb_unicode.c

extern/glfw/CMakeFiles/glfw.dir/src/xkb_unicode.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/xkb_unicode.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/xkb_unicode.c > CMakeFiles/glfw.dir/src/xkb_unicode.c.i

extern/glfw/CMakeFiles/glfw.dir/src/xkb_unicode.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/xkb_unicode.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/xkb_unicode.c -o CMakeFiles/glfw.dir/src/xkb_unicode.c.s

extern/glfw/CMakeFiles/glfw.dir/src/posix_time.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/posix_time.c.o: ../extern/glfw/src/posix_time.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/posix_time.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/posix_time.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/posix_time.c

extern/glfw/CMakeFiles/glfw.dir/src/posix_time.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/posix_time.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/posix_time.c > CMakeFiles/glfw.dir/src/posix_time.c.i

extern/glfw/CMakeFiles/glfw.dir/src/posix_time.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/posix_time.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/posix_time.c -o CMakeFiles/glfw.dir/src/posix_time.c.s

extern/glfw/CMakeFiles/glfw.dir/src/posix_thread.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/posix_thread.c.o: ../extern/glfw/src/posix_thread.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/posix_thread.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/posix_thread.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/posix_thread.c

extern/glfw/CMakeFiles/glfw.dir/src/posix_thread.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/posix_thread.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/posix_thread.c > CMakeFiles/glfw.dir/src/posix_thread.c.i

extern/glfw/CMakeFiles/glfw.dir/src/posix_thread.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/posix_thread.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/posix_thread.c -o CMakeFiles/glfw.dir/src/posix_thread.c.s

extern/glfw/CMakeFiles/glfw.dir/src/glx_context.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/glx_context.c.o: ../extern/glfw/src/glx_context.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/glx_context.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/glx_context.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/glx_context.c

extern/glfw/CMakeFiles/glfw.dir/src/glx_context.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/glx_context.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/glx_context.c > CMakeFiles/glfw.dir/src/glx_context.c.i

extern/glfw/CMakeFiles/glfw.dir/src/glx_context.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/glx_context.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/glx_context.c -o CMakeFiles/glfw.dir/src/glx_context.c.s

extern/glfw/CMakeFiles/glfw.dir/src/egl_context.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/egl_context.c.o: ../extern/glfw/src/egl_context.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/egl_context.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/egl_context.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/egl_context.c

extern/glfw/CMakeFiles/glfw.dir/src/egl_context.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/egl_context.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/egl_context.c > CMakeFiles/glfw.dir/src/egl_context.c.i

extern/glfw/CMakeFiles/glfw.dir/src/egl_context.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/egl_context.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/egl_context.c -o CMakeFiles/glfw.dir/src/egl_context.c.s

extern/glfw/CMakeFiles/glfw.dir/src/osmesa_context.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/osmesa_context.c.o: ../extern/glfw/src/osmesa_context.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_15) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/osmesa_context.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/osmesa_context.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/osmesa_context.c

extern/glfw/CMakeFiles/glfw.dir/src/osmesa_context.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/osmesa_context.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/osmesa_context.c > CMakeFiles/glfw.dir/src/osmesa_context.c.i

extern/glfw/CMakeFiles/glfw.dir/src/osmesa_context.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/osmesa_context.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/osmesa_context.c -o CMakeFiles/glfw.dir/src/osmesa_context.c.s

extern/glfw/CMakeFiles/glfw.dir/src/linux_joystick.c.o: extern/glfw/CMakeFiles/glfw.dir/flags.make
extern/glfw/CMakeFiles/glfw.dir/src/linux_joystick.c.o: ../extern/glfw/src/linux_joystick.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_16) "Building C object extern/glfw/CMakeFiles/glfw.dir/src/linux_joystick.c.o"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/glfw.dir/src/linux_joystick.c.o   -c /home/marlo/Documents/deepsea/box2d/extern/glfw/src/linux_joystick.c

extern/glfw/CMakeFiles/glfw.dir/src/linux_joystick.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glfw.dir/src/linux_joystick.c.i"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/box2d/extern/glfw/src/linux_joystick.c > CMakeFiles/glfw.dir/src/linux_joystick.c.i

extern/glfw/CMakeFiles/glfw.dir/src/linux_joystick.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glfw.dir/src/linux_joystick.c.s"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/box2d/extern/glfw/src/linux_joystick.c -o CMakeFiles/glfw.dir/src/linux_joystick.c.s

# Object files for target glfw
glfw_OBJECTS = \
"CMakeFiles/glfw.dir/src/context.c.o" \
"CMakeFiles/glfw.dir/src/init.c.o" \
"CMakeFiles/glfw.dir/src/input.c.o" \
"CMakeFiles/glfw.dir/src/monitor.c.o" \
"CMakeFiles/glfw.dir/src/vulkan.c.o" \
"CMakeFiles/glfw.dir/src/window.c.o" \
"CMakeFiles/glfw.dir/src/x11_init.c.o" \
"CMakeFiles/glfw.dir/src/x11_monitor.c.o" \
"CMakeFiles/glfw.dir/src/x11_window.c.o" \
"CMakeFiles/glfw.dir/src/xkb_unicode.c.o" \
"CMakeFiles/glfw.dir/src/posix_time.c.o" \
"CMakeFiles/glfw.dir/src/posix_thread.c.o" \
"CMakeFiles/glfw.dir/src/glx_context.c.o" \
"CMakeFiles/glfw.dir/src/egl_context.c.o" \
"CMakeFiles/glfw.dir/src/osmesa_context.c.o" \
"CMakeFiles/glfw.dir/src/linux_joystick.c.o"

# External object files for target glfw
glfw_EXTERNAL_OBJECTS =

bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/context.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/init.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/input.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/monitor.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/vulkan.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/window.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/x11_init.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/x11_monitor.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/x11_window.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/xkb_unicode.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/posix_time.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/posix_thread.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/glx_context.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/egl_context.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/osmesa_context.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/src/linux_joystick.c.o
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/build.make
bin/libglfw.a: extern/glfw/CMakeFiles/glfw.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/marlo/Documents/deepsea/box2d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_17) "Linking C static library ../../bin/libglfw.a"
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && $(CMAKE_COMMAND) -P CMakeFiles/glfw.dir/cmake_clean_target.cmake
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/glfw.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
extern/glfw/CMakeFiles/glfw.dir/build: bin/libglfw.a

.PHONY : extern/glfw/CMakeFiles/glfw.dir/build

extern/glfw/CMakeFiles/glfw.dir/clean:
	cd /home/marlo/Documents/deepsea/box2d/build/extern/glfw && $(CMAKE_COMMAND) -P CMakeFiles/glfw.dir/cmake_clean.cmake
.PHONY : extern/glfw/CMakeFiles/glfw.dir/clean

extern/glfw/CMakeFiles/glfw.dir/depend:
	cd /home/marlo/Documents/deepsea/box2d/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/marlo/Documents/deepsea/box2d /home/marlo/Documents/deepsea/box2d/extern/glfw /home/marlo/Documents/deepsea/box2d/build /home/marlo/Documents/deepsea/box2d/build/extern/glfw /home/marlo/Documents/deepsea/box2d/build/extern/glfw/CMakeFiles/glfw.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : extern/glfw/CMakeFiles/glfw.dir/depend

