################################################################################
#
# Copyright (c) 2024, UnaBiz SAS
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#  1 Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#  2 Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  3 Neither the name of UnaBiz SAS nor the names of its contributors may be
#    used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
# THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
# CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
################################################################################

cmake_minimum_required(VERSION 3.15)
project(sigfox_ep_addon_ta)
    
#Check if Project configure by himself or by a parent project (like sigfox-ep-lib)
if ( ${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR} )
    if (NOT DEFINED  SIGFOX_EP_LIB_DIR)
        message(FATAL_ERROR "${PROJECT_NAME} cannot be configured without -DSIGFOX_EP_LIB_DIR=<sigfox-ep-lib directory>")
    endif()
    set(ADDON_TA OFF)
    add_subdirectory(${SIGFOX_EP_LIB_DIR} "${CMAKE_CURRENT_BINARY_DIR}/sigfox-ep-lib")
else()
    if (NOT DEFINED SIGFOX_EP_LIB_DIR)
        set(SIGFOX_EP_LIB_DIR ${CMAKE_SOURCE_DIR})
        message("${PROJECT_NAME} is Fetched by a parent project")
    endif()
endif()

set(ADDON_TA_SOURCES
    src/sigfox_ep_addon_ta_api.c
    src/test_modes_ta/sigfox_ep_addon_ta_cw.c
    src/test_modes_ta/sigfox_ep_addon_ta_csul.c
    src/test_modes_ta/sigfox_ep_addon_ta_csdl.c
)

set(ADDON_TA_HEADERS 
    inc/sigfox_ep_addon_ta_api.h
    inc/sigfox_ep_addon_ta_version.h
    inc/test_modes_ta/sigfox_ep_addon_ta_cw.h
    inc/test_modes_ta/sigfox_ep_addon_ta_csul.h
    inc/test_modes_ta/sigfox_ep_addon_ta_csdl.h
)

set(ADDON_TA_PUBLIC_HEADERS
    inc/sigfox_ep_addon_ta_api.h
    inc/sigfox_ep_addon_ta_version.h
)

if(${SIGFOX_EP_BIDIRECTIONAL} STREQUAL OFF)
    list(REMOVE_ITEM ADDON_TA_SOURCES "src/test_modes_ta/sigfox_ep_addon_ta_csdl.c")
endif()

#Add Cmake module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

#Precompile module
find_program(UNIFDEF unifdef)
if(NOT UNIFDEF)
    message(STATUS "unifdef - not found (precompilation target will not be available)")   
else()
    include(precompile_addon_ta)
endif()

#Target to create object
add_library(${PROJECT_NAME}_obj OBJECT EXCLUDE_FROM_ALL ${ADDON_TA_SOURCES})
target_include_directories( ${PROJECT_NAME}_obj PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    PRIVATE
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:sigfox_ep_lib_obj,INTERFACE_INCLUDE_DIRECTORIES>>
)

#Target to create library
add_library(${PROJECT_NAME} STATIC $<TARGET_OBJECTS:${PROJECT_NAME}_obj>)
target_include_directories(${PROJECT_NAME} PUBLIC 
    ${SIGFOX_EP_LIB_DIR}/inc 
    PRIVATE
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:sigfox_ep_lib_obj,INTERFACE_INCLUDE_DIRECTORIES>>
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    PUBLIC_HEADER "${ADDON_TA_PUBLIC_HEADERS}"
    ARCHIVE_OUTPUT_DIRECTORY ${LIB_LOCATION}
    LIBRARY_OUTPUT_DIRECTORY ${LIB_LOCATION}
)
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${LIB_LOCATION}
    PUBLIC_HEADER DESTINATION ${API_LOCATION}  
)
