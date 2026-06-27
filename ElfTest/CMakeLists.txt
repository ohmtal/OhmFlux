set(ELF_DIR "${CMAKE_CURRENT_LIST_DIR}")

#--------------------------


set(ELFSCRIPT_PATH "/opt/ElfScript" CACHE PATH "Path to ElfScript")
set(ELF_MODULE ${ELFSCRIPT_PATH}/ElfScript)
set(ELF_CMAKE ${ELF_MODULE}/ElfScript.cmake)
# set(ELF_MODULE ${ELFSCRIPT_PATH}/TorqueScript)
# set(ELF_CMAKE ${ELF_MODULE}/TorqueScript.cmake)

if(NOT EXISTS ${ELF_CMAKE})
    message(FATAL_ERROR
        "\n TorqueScript path is not set! "
        "\n Please provide the path using: "
        "\n cmake -B build -DELFSCRIPT_PATH=/path/to/ElfScript"
    )
else()
 #  defines TORQUESCRIPT_SRC
 include(${ELF_CMAKE})

  set (ELFSCRIPT_INCLUDES
   ${ELF_MODULE}
  )
endif()


# ~~~ BOX2D (legacy version from OGE)~~~~
    include("${ELF_DIR}/src/Box2D/Box2D.cmake")
# -------------------
set(PROJECT_SOURCES

    # FIXME port sources

    # --- custom sources ---
    ${ELF_DIR}/src/main.cpp
    ${ELF_DIR}/src/appMain.h
    ${ELF_DIR}/src/appMain.cpp
    ${ELF_DIR}/src/myPlatformProcess.cc

    ${ELF_DIR}/src/b2b/b2btest.cc
    ${ELF_DIR}/src/b2b/b2btest.h
    ${ELF_DIR}/src/b2b/b2Objects.cc
    ${ELF_DIR}/src/b2b/b2Objects.h
    ${ELF_DIR}/src/b2b/b2Joints.cc
    ${ELF_DIR}/src/b2b/b2Joints.h

    ${ELF_DIR}/src/core/GameCtrl.h
    ${ELF_DIR}/src/core/GameCtrl.cpp
    ${ELF_DIR}/src/core/Grid.cpp
    ${ELF_DIR}/src/core/Grid.h
    ${ELF_DIR}/src/core/Utility.cpp
    ${ELF_DIR}/src/core/Utility.h


    ${ELF_DIR}/src/consoleFunctions/app.cpp
    ${ELF_DIR}/src/consoleFunctions/objects.cpp
    ${ELF_DIR}/src/consoleFunctions/ConsoleTypes.cpp
    ${ELF_DIR}/src/consoleFunctions/ConsoleTypes.h


    ${ELF_DIR}/src/audio/AudioProfile.h
    ${ELF_DIR}/src/audio/AudioProfile.cpp

    ${ELF_DIR}/src/render/Camera.cpp
    ${ELF_DIR}/src/render/Camera.h
    ${ELF_DIR}/src/render/Font.h
    ${ELF_DIR}/src/render/Font.cpp
    ${ELF_DIR}/src/render/Label.h
    ${ELF_DIR}/src/render/Label.cpp
    ${ELF_DIR}/src/render/Light.cpp
    ${ELF_DIR}/src/render/Light.h
    ${ELF_DIR}/src/render/Particle.cpp
    ${ELF_DIR}/src/render/Particle.h
    ${ELF_DIR}/src/render/Sprite.h
    ${ELF_DIR}/src/render/Sprite.cpp
    ${ELF_DIR}/src/render/Texture.h
    ${ELF_DIR}/src/render/Texture.cpp

    # 2FA i dont need this but i have it so ...
    ${ELF_DIR}/src/totp/base32.cc
    ${ELF_DIR}/src/totp/base32.h
    ${ELF_DIR}/src/totp/hmac-sha1.cc
    ${ELF_DIR}/src/totp/hmac-sha1.h
    ${ELF_DIR}/src/totp/totp.cc



    # # basic math move to TorqueScript
    # ${ELF_DIR}/src/math/mMathFn.h
    # ${ELF_DIR}/src/math/mConsoleFunctions.cpp


    # ---- editor
    ${ELF_DIR}/src/scriptEditor/scriptEditor.cpp
    ${ELF_DIR}/src/scriptEditor/scriptEditor.h
    ${ELF_DIR}/src/scriptEditor/ImGuiColorTextEdit/TextEditor.cpp
    ${ELF_DIR}/src/scriptEditor/ImGuiColorTextEdit/TextEditor.h

    # --- testing
    ${ELF_DIR}/src/test/testObj.cpp

    ${BOX2D_SRCS}
    # ${TORQUESCRIPT_SRC}
    ${ELFSCRIPT_SRC}
)


configure_project_target(ElfTest ${ELF_DIR} "${ELF_DIR}/assets@assets" "${PROJECT_SOURCES}")


target_include_directories(ElfTest PRIVATE ${ELF_DIR}/src)
target_include_directories(ElfTest PRIVATE ${ELFSCRIPT_INCLUDES})

