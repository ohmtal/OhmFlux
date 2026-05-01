set(AUDIOTESTBED_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(GAME_SOURCES
    ${AUDIOTESTBED_DIR}/src/main.cpp
    ${AUDIOTESTBED_DIR}/src/audio/AudioResourceManager.cpp
    ${AUDIOTESTBED_DIR}/src/audio/AudioInstance.cpp
)


configure_project_target(AudioTestBed ${AUDIOTESTBED_DIR} "${AUDIOTESTBED_DIR}/assets@assets" "${GAME_SOURCES}")

