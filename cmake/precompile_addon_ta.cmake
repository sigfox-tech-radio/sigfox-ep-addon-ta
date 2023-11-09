find_program(UNIFDEF unifdef REQUIRED)
if(NOT UNIFDEF)
    message(FATAL_ERROR "unifdef not found!")
endif()

#List of precompileInc and precompileSrc files
foreach(X IN LISTS ADDON_TA_SOURCES)
	LIST(APPEND PRECOMPIL_ADDON_TA_SOURCES "${PRECOMPIL_DIR}/${X}")
endforeach()
foreach(X IN LISTS ADDON_TA_HEADERS)
	LIST(APPEND PRECOMPIL_ADDON_TA_HEADERS "${PRECOMPIL_DIR}/${X}")
endforeach()
foreach(X IN LISTS ADDON_TA_PUBLIC_HEADERS)
	LIST(APPEND PRECOMPIL_ADDON_TA_PUBLIC_HEADERS "${PRECOMPIL_DIR}/${X}")
endforeach()

#Custom command Loop for all Sources
foreach(X IN LISTS ADDON_TA_SOURCES ADDON_TA_HEADERS)
add_custom_command(
	OUTPUT "${PRECOMPIL_DIR}/${X}"
	DEPENDS ${CMAKE_BINARY_DIR}/undefs_file
	DEPENDS ${CMAKE_BINARY_DIR}/defs_file
    DEPENDS ${X}
	COMMAND	${CMAKE_COMMAND} -E make_directory ${PRECOMPIL_DIR}/src/test_modes_ta  ${PRECOMPIL_DIR}/inc/test_modes_ta
    COMMAND unifdef -B -k -x 2 -f ${CMAKE_BINARY_DIR}/undefs_file -f ${CMAKE_BINARY_DIR}/defs_file ${PROJECT_SOURCE_DIR}/${X} > "${PRECOMPIL_DIR}/${X}" 
	VERBATIM
)

endforeach()
set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS 1)

add_custom_target(precompil_${PROJECT_NAME}
	DEPENDS precompil
    DEPENDS ${PRECOMPIL_ADDON_TA_SOURCES}
    DEPENDS ${PRECOMPIL_ADDON_TA_HEADERS}
  	VERBATIM
)
