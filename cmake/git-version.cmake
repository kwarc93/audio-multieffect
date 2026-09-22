# =============================================================================
# Determines the firmware version string from git (equivalent of
# makefile.defs' `GIT_REVISION := $(shell git describe --tags --dirty --always)`),
# exposed to C/C++ via the GIT_REVISION define.
#
# Evaluated at configure time; re-run cmake configure to refresh it.
# =============================================================================

function(get_git_revision OUT_VAR)
    find_package(Git QUIET)

    set(_git_revision "unknown")

    if(GIT_EXECUTABLE)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --always
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE _describe_output
            RESULT_VARIABLE _describe_result
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(_describe_result EQUAL 0 AND _describe_output)
            set(_git_revision "${_describe_output}")
        endif()
    else()
        message(WARNING "git executable not found - GIT_REVISION will be \"unknown\"")
    endif()

    set(${OUT_VAR} "${_git_revision}" PARENT_SCOPE)
endfunction()
