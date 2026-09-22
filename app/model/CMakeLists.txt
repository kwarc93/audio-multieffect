# =============================================================================
# DSP effect chain: the generic processor plus one subdirectory per effect
# type. NAM (Neural Amp Modeler) is optional - excluded on boards where
# CFG_DISABLE_NEURAL_AMP_MODELER applies (F7) or where app/model isn't built
# at all (dual-core CM4/UI core - see HAS_MODEL in the top-level CMakeLists).
# =============================================================================

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/effect_processor.cpp
)

add_subdirectory(amp_sim)
add_subdirectory(cabinet_sim)
add_subdirectory(chorus)
add_subdirectory(echo)
add_subdirectory(overdrive)
add_subdirectory(phaser)
add_subdirectory(reverb)
add_subdirectory(tremolo)
add_subdirectory(tuner)
add_subdirectory(vocoder)

if(HAS_MODEL_NAM)
    add_subdirectory(nam)
endif()
