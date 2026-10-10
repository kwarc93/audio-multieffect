# =============================================================================
# Generates a source-annotated firmware disassembly listing.
#
# Invoked by the `listing` target with -D arguments for the objdump executable,
# the built ELF firmware image, and the destination listing file.
# =============================================================================

foreach(required_variable OBJDUMP ELF LISTING)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} must be provided")
    endif()
endforeach()

execute_process(
    COMMAND "${OBJDUMP}" --source --all-headers --demangle --line-numbers --wide "${ELF}"
    OUTPUT_FILE "${LISTING}"
    RESULT_VARIABLE listing_result
)

if(NOT listing_result EQUAL 0)
    message(FATAL_ERROR "objdump failed while generating '${LISTING}' (exit code ${listing_result})")
endif()
