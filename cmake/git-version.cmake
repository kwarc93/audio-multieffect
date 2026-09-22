# =============================================================================
# Determine firmware version information from git, mirroring what
# makefile.defs used to do for the Eclipse/make build:
#
#   GIT_REVISION := $(shell git describe --tags --dirty --always)
#
# The result is exposed to C/C++ as the GIT_REVISION macro (a quoted string),
# exactly like the old build did with -DGIT_REVISION='"..."'.
#
# This runs at *configure* time (when you run "cmake --preset ..."), not at
# every build, so re-run cmake configure (or just re-build - see the
# add_custom_target re-check below) if you want the string refreshed after
# committing.
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
