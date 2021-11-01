# CMakeLists.txt
#
# Top-level CMake build file for the 'sockpp' library.
#
# ---------------------------------------------------------------------------
# This file is part of the "sockpp" C++ socket library.
#
# Copyright (c) 2017-2018 Frank Pagliughi
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
# contributors may be used to endorse or promote products derived from this
# software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
# IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ---------------------------------------------------------------------------

## project
project(sockpp VERSION "0.7.0")

## library name
set(SOCKPP sockpp)
set(SOCKPP_STATIC ${SOCKPP}-static)

# --- Generate a version header ---

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/version.h.in  
	generated/${SOCKPP}/version.h
	@ONLY
)

# --- Common Library Sources, etc ---

add_subdirectory(src)

# --- Header Locations ---

target_include_directories(sockpp-objs 
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE 
		${CMAKE_CURRENT_BINARY_DIR}/generated
		src
)

# --- System libraries ---

include(GNUInstallDirs)

if(WIN32)
	set(LIBS_SYSTEM ws2_32)
endif()

# --- Create the static library ---

message(STATUS "Creating static library: ${SOCKPP_STATIC}")

add_library(${SOCKPP_STATIC} STATIC $<TARGET_OBJECTS:sockpp-objs>)

## add dependencies to the static library
target_link_libraries(${SOCKPP_STATIC} ${LIBS_SYSTEM} project_options)

target_include_directories(${SOCKPP_STATIC} 
	PUBLIC 
		$<TARGET_PROPERTY:sockpp-objs,INCLUDE_DIRECTORIES>
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/generated
)
