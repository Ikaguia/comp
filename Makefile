#**************************************************************************************************
#
#   raylib makefile for Desktop platforms, Raspberry Pi, Android and HTML5
#
#   Copyright (c) 2013-2019 Ramon Santamaria (@raysan5)
#
#   This software is provided "as-is", without any express or implied warranty. In no event
#   will the authors be held liable for any damages arising from the use of this software.
#
#   Permission is granted to anyone to use this software for any purpose, including commercial
#   applications, and to alter it and redistribute it freely, subject to the following restrictions:
#
#     1. The origin of this software must not be misrepresented; you must not claim that you
#     wrote the original software. If you use this software in a product, an acknowledgment
#     in the product documentation would be appreciated but is not required.
#
#     2. Altered source versions must be plainly marked as such, and must not be misrepresented
#     as being the original software.
#
#     3. This notice may not be removed or altered from any source distribution.
#
#**************************************************************************************************
#
# This is an altered source version.
#
#**************************************************************************************************


.PHONY: all clean

# Define variables
PROJECT_NAME       ?= comp

# Define compiler path on Windows
COMPILER_PATH      ?= C:/w64devkit/bin

# Define default options
# One of PLATFORM_DESKTOP, PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
PLATFORM           ?= PLATFORM_DESKTOP

# Locations of your newly installed library and associated headers. See ../src/Makefile
# On Linux, if you have installed raylib but cannot compile the examples, check that
# the *_INSTALL_PATH values here are the same as those in src/Makefile or point to known locations.
# To enable system-wide compile-time and runtime linking to libraylib.so, run ../src/$ sudo make install RAYLIB_LIBTYPE_SHARED.
# To enable compile-time linking to a special version of libraylib.so, change these variables here.
# To enable runtime linking to a special version of libraylib.so, see EXAMPLE_RUNTIME_PATH below.
# If there is a libraylib in both EXAMPLE_RUNTIME_PATH and RAYLIB_INSTALL_PATH, at runtime,
# the library at EXAMPLE_RUNTIME_PATH, if present, will take precedence over the one at RAYLIB_INSTALL_PATH.
# RAYLIB_INSTALL_PATH should be the desired full path to libraylib. No relative paths.
DESTDIR ?= /usr/local

# Build mode for project: DEBUG or RELEASE
BUILD_MODE            ?= RELEASE

# Determine PLATFORM_OS in case PLATFORM_DESKTOP selected
PLATFORM_OS=WINDOWS
export PATH := $(COMPILER_PATH):$(PATH)

# Define default C compiler: gcc
# NOTE: define g++ compiler if using C++
CC = g++

# Define default make program: Mingw32-make
MAKE = mingw32-make

# Define compiler flags:
#  -O0                  defines optimization level (no optimization, better for debugging)
#  -O1                  defines optimization level
#  -g                   include debug information on compilation
#  -s                   strip unnecessary data from build -> do not use in debug builds
#  -Wall                turns on most, but not all, compiler warnings
#  -std=c99             defines C language mode (standard C from 1999 revision)
#  -std=gnu99           defines C language mode (GNU C from 1999 revision)
#  -Wno-missing-braces  ignore invalid warning (GCC bug 53119)
#  -D_DEFAULT_SOURCE    use with -std=c99 on Linux and PLATFORM_WEB, required for timespec
CFLAGS += -Wall -std=c++23 -D_DEFAULT_SOURCE -Wno-missing-braces -Wshadow -Wextra -Werror
CFLAGS += -MMD -MP     #recompile .cpp when included .hpp are changed.

ifeq ($(BUILD_MODE),DEBUG)
	CFLAGS += -g -O0 -DDEBUG_BUILD
else
	CFLAGS += -s -O1 -DRELEASE_BUILD
endif

# Define include paths for required headers
# NOTE: Several external required libraries (stb and others)
INCLUDE_PATHS = -I. -I./include
# INCLUDE_PATHS += -isystem ./external

# Define any libraries required on linking
# if you want to link libraries (libname.so or libname.a), use the -lname
LDLIBS = -lstdc++exp

# Additional flags for compiler (if desired)
#CFLAGS += -Wextra -Wmissing-prototypes -Wstrict-prototypes
# resource file contains windows executable icon and properties
# -Wl,--subsystem,windows hides the console window
LDFLAGS += -Wl,--subsystem,windows

# Define a recursive wildcard function
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

# Define all source files required
SRC_DIR = src
OBJ_DIR = obj

# Define all object files from source files
SRC = $(call rwildcard, ${SRC_DIR}, *.cpp)
OBJS ?= $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

MAKEFILE_PARAMS = $(PROJECT_NAME)

ifeq ($(OS),Windows_NT)
	MKDIR_P = if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
else
	MKDIR_P = mkdir -p $(OBJ_DIR)
endif

# Default target entry
all: $(PROJECT_NAME)

# Project target defined by PROJECT_NAME
$(PROJECT_NAME): $(OBJS)
	$(CC) -o $(PROJECT_NAME)$(EXT) $(OBJS) $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -D$(PLATFORM)

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(MKDIR_P)
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM)

# Clean everything
clean:
	del *.o *.exe /s
	@echo Cleaning done

-include $(OBJS:.o=.d)
