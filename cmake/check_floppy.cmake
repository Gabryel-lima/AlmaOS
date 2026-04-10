if(NOT DEFINED MAKE_EXECUTABLE)
    message(FATAL_ERROR "MAKE_EXECUTABLE is required")
endif()

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

if(NOT DEFINED BUILD_DIR)
    message(FATAL_ERROR "BUILD_DIR is required")
endif()

file(REAL_PATH "${SOURCE_DIR}/${BUILD_DIR}" BUILD_DIR_ABS)

if(NOT DEFINED MDIR_EXECUTABLE)
    message(FATAL_ERROR "MDIR_EXECUTABLE is required")
endif()

execute_process(
    COMMAND ${MAKE_EXECUTABLE} -C ${SOURCE_DIR} BUILD_DIR=${BUILD_DIR}
    RESULT_VARIABLE build_result
    OUTPUT_VARIABLE build_output
    ERROR_VARIABLE build_error)

if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "make failed:\n${build_output}\n${build_error}")
endif()

execute_process(
    COMMAND ${MDIR_EXECUTABLE} -i ${BUILD_DIR_ABS}/floppy.img ::/
    RESULT_VARIABLE list_result
    OUTPUT_VARIABLE list_output
    ERROR_VARIABLE list_error)

if(NOT list_result EQUAL 0)
    message(FATAL_ERROR "mdir failed:\n${list_output}\n${list_error}")
endif()

string(TOLOWER "${list_output}" list_lower)

foreach(required_name IN ITEMS stage2 kernel test)
    string(FIND "${list_lower}" "${required_name}" required_pos)
    if(required_pos EQUAL -1)
        message(FATAL_ERROR "Missing required file in floppy image: ${required_name}")
    endif()
endforeach()

string(FIND "${list_lower}" "protect" protect_pos)
if(NOT protect_pos EQUAL -1)
    message(FATAL_ERROR "The protected-mode demo must not be packaged in the default image")
endif()
