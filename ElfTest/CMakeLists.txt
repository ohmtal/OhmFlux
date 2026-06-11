set(ELF_DIR "${CMAKE_CURRENT_LIST_DIR}")

#--------------------------


set(ELFSCRIPT_PATH "/opt/TorqueScript" CACHE PATH "Path to TorqueScript")

if(NOT EXISTS "${ELFSCRIPT_PATH}/TorqueScript/TorqueScript.cmake")
    message(FATAL_ERROR
        "\n TorqueScript path is not set! "
        "\n Please provide the path using: "
        "\n cmake -B build -DELFSCRIPT_PATH=/path/to/TorqueScript"
    )
else()
 #  defines TORQUESCRIPT_SRC
 include("${ELFSCRIPT_PATH}/TorqueScript/TorqueScript.cmake")

  set (ELFSCRIPT_INCLUDES
    ${ELFSCRIPT_PATH}/TorqueScript
  )
endif()




# ~~~ BOX2D (legacy version from OGE)~~~~
set(BOX2D_PATH  "${CMAKE_CURRENT_LIST_DIR}/src")
set(BOX2D_SRCS
    ${BOX2D_PATH}/Box2D/Collision/b2BroadPhase.cpp
    ${BOX2D_PATH}/Box2D/Collision/b2CollideCircle.cpp
    ${BOX2D_PATH}/Box2D/Collision/b2CollideEdge.cpp
    ${BOX2D_PATH}/Box2D/Collision/b2CollidePolygon.cpp
    ${BOX2D_PATH}/Box2D/Collision/b2Collision.cpp
    ${BOX2D_PATH}/Box2D/Collision/b2Distance.cpp
    ${BOX2D_PATH}/Box2D/Collision/b2DynamicTree.cpp
    ${BOX2D_PATH}/Box2D/Collision/b2TimeOfImpact.cpp
    ${BOX2D_PATH}/Box2D/Collision/Shapes/b2ChainShape.cpp
    ${BOX2D_PATH}/Box2D/Collision/Shapes/b2CircleShape.cpp
    ${BOX2D_PATH}/Box2D/Collision/Shapes/b2EdgeShape.cpp
    ${BOX2D_PATH}/Box2D/Collision/Shapes/b2PolygonShape.cpp
    ${BOX2D_PATH}/Box2D/Common/b2BlockAllocator.cpp
    ${BOX2D_PATH}/Box2D/Common/b2Draw.cpp
    ${BOX2D_PATH}/Box2D/Common/b2FreeList.cpp
    ${BOX2D_PATH}/Box2D/Common/b2Math.cpp
    ${BOX2D_PATH}/Box2D/Common/b2Settings.cpp
    ${BOX2D_PATH}/Box2D/Common/b2StackAllocator.cpp
    ${BOX2D_PATH}/Box2D/Common/b2Timer.cpp
    ${BOX2D_PATH}/Box2D/Common/b2TrackedBlock.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/b2Body.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/b2ContactManager.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/b2Fixture.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/b2Island.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/b2World.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/b2WorldCallbacks.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Contacts/b2CircleContact.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Contacts/b2Contact.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Contacts/b2ContactSolver.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Contacts/b2PolygonContact.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Joints/b2DistanceJoint.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Joints/b2FrictionJoint.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Joints/b2GearJoint.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Joints/b2Joint.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Joints/b2MotorJoint.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Joints/b2MouseJoint.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Joints/b2PrismaticJoint.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Joints/b2PulleyJoint.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Joints/b2RevoluteJoint.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Joints/b2RopeJoint.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Joints/b2WeldJoint.cpp
    ${BOX2D_PATH}/Box2D/Dynamics/Joints/b2WheelJoint.cpp
    ${BOX2D_PATH}/Box2D/Rope/b2Rope.cpp
    ${BOX2D_PATH}/Box2D/Particle/b2Particle.cpp
    ${BOX2D_PATH}/Box2D/Particle/b2ParticleAssembly.cpp
    ${BOX2D_PATH}/Box2D/Particle/b2ParticleGroup.cpp
    ${BOX2D_PATH}/Box2D/Particle/b2ParticleSystem.cpp
    ${BOX2D_PATH}/Box2D/Particle/b2VoronoiDiagram.cpp
)

# -------------------
set(ELF_DIR ./)

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
    ${ELF_DIR}/src/core/ConsoleTypes.cpp
    ${ELF_DIR}/src/core/ConsoleTypes.h
    ${ELF_DIR}/src/core/Grid.cpp
    ${ELF_DIR}/src/core/Grid.h
    ${ELF_DIR}/src/core/Utility.cpp
    ${ELF_DIR}/src/core/Utility.h


    ${ELF_DIR}/src/consoleFunctions/app.cpp
    ${ELF_DIR}/src/consoleFunctions/math.cpp

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



    # basic math
    ${ELF_DIR}/src/math/mMathFn.h
    ${ELF_DIR}/src/math/mConsoleFunctions.cpp


    # ---- editor
    ${ELF_DIR}/src/scriptEditor/scriptEditor.cpp
    ${ELF_DIR}/src/scriptEditor/scriptEditor.h
    ${ELF_DIR}/src/scriptEditor/ImGuiColorTextEdit/TextEditor.cpp
    ${ELF_DIR}/src/scriptEditor/ImGuiColorTextEdit/TextEditor.h


    ${BOX2D_SRCS}
    ${TORQUESCRIPT_SRC}
)


configure_project_target(ElfTestBed ${ELF_DIR} "${ELF_DIR}/assets@assets" "${PROJECT_SOURCES}")

# target_compile_definitions(KorkTestBed PUBLIC TORQUE_ENABLE_ASSERTS)


target_include_directories(ElfTestBed PRIVATE ${ELF_DIR}/src)
target_include_directories(ElfTestBed PRIVATE ${ELFSCRIPT_INCLUDES})

# set flag in main cmake
# target_compile_options(ElfTestBed PRIVATE -fsanitize=address -g)
# target_link_options(ElfTestBed PRIVATE -fsanitize=address)
