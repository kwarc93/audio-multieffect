# nam.cpp wraps libs/nam-core (Neural Amp Modeler); the third-party core
# library itself is added in libs/nam-core/CMakeLists.txt.
target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/nam.cpp)
