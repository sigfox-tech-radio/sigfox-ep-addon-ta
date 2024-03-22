cmake_minimum_required(VERSION 3.15)
project(sigfox_ep_addon_ta)
    
#Check if Project configure by himself or by a parent project (like sigfox-ep-lib)
if ( ${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR} )
    if (NOT DEFINED  SIGFOX_EP_LIB_DIR)
        message(FATAL_ERROR "${PROJECT_NAME} cannot be configured without -DSIGFOX_EP_LIB_DIR=<sigfox-ep-lib directory>")
    endif()
    set(ADDON_TA OFF)
    add_subdirectory(${SIGFOX_EP_LIB_DIR} "${CMAKE_CURRENT_BINARY_DIR}/${SIGFOX_EP_LIB_DIR}") 
else()
    set(SIGFOX_EP_LIB_DIR ${CMAKE_SOURCE_DIR})
    message("${PROJECT_NAME} is Fetched by a parent project")
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

#When sigfox_ep_flag.h is don't used
if(${USE_SIGFOX_EP_FLAGS_H} STREQUAL "ON")
else()
    if(${BIDIRECTIONAL} STREQUAL OFF)
        list(REMOVE_ITEM ADDON_TA_SOURCES "src/test_modes_ta/sigfox_ep_addon_ta_csdl.c")
    endif()
endif()

#Add Cmake module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

#Precompile module
include(precompile_addon_ta)

add_library(${PROJECT_NAME}_obj OBJECT EXCLUDE_FROM_ALL ${ADDON_TA_SOURCES})
target_include_directories( ${PROJECT_NAME}_obj PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${SIGFOX_EP_LIB_DIR}/inc
)
target_compile_definitions(${PROJECT_NAME}_obj PUBLIC ${DEF_FLAG_LIST})

add_library(${PROJECT_NAME} STATIC ${PRECOMPIL_ADDON_TA_SOURCES})
add_dependencies(${PROJECT_NAME}  precompil_${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC ${PRECOMPIL_DIR}/inc)
target_compile_definitions(${PROJECT_NAME} PUBLIC ${DEF_FLAG_LIST})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PRECOMPIL_ADDON_TA_PUBLIC_HEADERS}")
set_target_properties(${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${LIB_LOCATION}
    LIBRARY_OUTPUT_DIRECTORY ${LIB_LOCATION}
)
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${LIB_LOCATION}
    PUBLIC_HEADER DESTINATION ${API_LOCATION}  
)
