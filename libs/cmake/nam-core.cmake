# =============================================================================
# Neural Amp Modeler core (libs/nam-core, git submodule "nam-pedal").
#
# nam-core is a git submodule pinned to an upstream commit and must stay
# unmodified - so this file lives outside the submodule and just points at
# its sources, instead of adding a CMakeLists.txt inside it. Upstream ships
# no CMake support of its own.
#
# Only pulled in when HAS_MODEL_NAM is ON (see top-level CMakeLists.txt);
# disabled on boards built with CFG_DISABLE_NEURAL_AMP_MODELER, and unused on
# the dual-core CM4 (UI/storage) core.
#
# NOTE: NAMPedal.cpp is intentionally NOT compiled (it was excluded in the
# original Eclipse project too - it's an alternate/example entry point, our
# app/model/nam wrapper talks to nam_model.c directly).
# =============================================================================

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../nam-core/nam_model.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../nam-core
)
