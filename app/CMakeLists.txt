# =============================================================================
# Application layer. Which parts get built depends on the board's role
# (full single-core app, dual-core UI/storage core, or dual-core DSP core) -
# see the HAS_* variables set in the top-level CMakeLists.txt.
# =============================================================================

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# app/ipc/*.hpp is header-only (included directly by main.cpp) - nothing to
# add_subdirectory() there, it just needs the repo root on the include path,
# which is already set up in the top-level CMakeLists.txt.

if(HAS_CONTROLLER)
    add_subdirectory(controller)
endif()

if(HAS_MODEL)
    add_subdirectory(model)
endif()

# app/modules (settings/presets managers) is used by both the full app and
# the dual-core UI/storage (CM4) core.
if(HAS_CONTROLLER)
    add_subdirectory(modules)
endif()

if(HAS_VIEW)
    add_subdirectory(view)
endif()
