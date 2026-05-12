set(AUDIOTESTBED_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(PROJECT_SOURCES
    ${AUDIOTESTBED_DIR}/src/main.cpp
    ${AUDIOTESTBED_DIR}/src/audio/AudioResourceManager.cpp
    ${AUDIOTESTBED_DIR}/src/audio/AudioInstance.cpp

    ${SFX_DIR}/SFXGeneratorStereo.cpp
)


configure_project_target(AudioTestBed ${AUDIOTESTBED_DIR} "${AUDIOTESTBED_DIR}/assets@assets" "${PROJECT_SOURCES}")
target_compile_definitions(AudioTestBed PRIVATE AUDIO_PORT_SFX)
