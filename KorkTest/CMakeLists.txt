set(KORK_DIR "${CMAKE_CURRENT_LIST_DIR}")

#--------------------------

# ----------------- korkscript Test
# option(ENABLE_KS "Enable korkscript" ON)

set(KS_PATH "/opt/korkscript" CACHE PATH "Path to KorkScript")

if(NOT EXISTS "${KS_PATH}/CMakeLists.txt")
    message(FATAL_ERROR
        "\n KorkScript path is not set! "
        "\n Please provide the path using: "
        "\n cmake -B build -DKS_PATH=/path/to/KorkScript"
    )
else()

  # removed submodule!
  # find_package(Git REQUIRED)
  #
  # # Check if the submodule's source files exist
  # if(NOT EXISTS "${$KS_PATH}/CMakeLists.txt" )
  #     message(STATUS "Initializing Git submodules...")
  #     execute_process(
  #         COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
  #         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  #         RESULT_VARIABLE GIT_SUBMODULE_RESULT
  #     )
  #
  #     if(NOT GIT_SUBMODULE_RESULT EQUAL 0)
  #         message(FATAL_ERROR "git submodule update failed with code ${GIT_SUBMODULE_RESULT}")
  #     endif()
  # endif()



  set (KS_INCLUDES
    ${KS_PATH}/
    ${KS_PATH}/engine
    ${KS_PATH}/torqueSim
  )

  set(KS_SRCS
      ${KS_PATH}/engine/core/findMatch.cc
      ${KS_PATH}/engine/core/hashFunction.cc
      ${KS_PATH}/engine/core/memStream.cc
      ${KS_PATH}/engine/core/nStream.cc
      ${KS_PATH}/engine/core/escape.cc

      ${KS_PATH}/engine/platform/platform.cc
      ${KS_PATH}/engine/platform/platformAssert.cc

      ${KS_PATH}/engine/console/compiledEval.cc
      ${KS_PATH}/engine/console/codeBlock.cc
      ${KS_PATH}/engine/console/consoleDoc.cc
      ${KS_PATH}/engine/console/stringStack.cc
      ${KS_PATH}/engine/console/consoleInternal.cc
      ${KS_PATH}/engine/console/consoleNamespace.cc
      ${KS_PATH}/engine/console/astNodes.cc
      ${KS_PATH}/engine/console/astAlloc.cc
      ${KS_PATH}/engine/console/compiler.cc

      ${KS_PATH}/engine/console/telnetConsole.cc
      ${KS_PATH}/engine/console/telnetDebugger.cc

      ${KS_PATH}/engine/console/consoleValue.h
      ${KS_PATH}/engine/console/simpleLexer.h
      ${KS_PATH}/engine/console/simpleParser.h
      ${KS_PATH}/engine/embed/api.h
      ${KS_PATH}/engine/embed/api.cpp
  )

  set(TORQUESIM_SRCS
      ${KS_PATH}/torqueSim/core/idGenerator.cc
      ${KS_PATH}/torqueSim/core/fileStream.cc
      ${KS_PATH}/torqueSim/core/stringUnit.cc
      ${KS_PATH}/torqueSim/core/stringTable.cc
      ${KS_PATH}/torqueSim/core/unicode.cc
      ${KS_PATH}/torqueSim/core/unicode.h
      ${KS_PATH}/torqueSim/console/console.cc
      ${KS_PATH}/torqueSim/console/console.h
      ${KS_PATH}/torqueSim/console/consoleFunctions.cc
      ${KS_PATH}/torqueSim/console/consoleLogger.cc
      ${KS_PATH}/torqueSim/console/consoleLogger.h
      ${KS_PATH}/torqueSim/console/consoleObject.cc
      ${KS_PATH}/torqueSim/console/consoleObject.h
      ${KS_PATH}/torqueSim/console/consoleTypes.cc
      ${KS_PATH}/torqueSim/console/consoleTypes.h
      ${KS_PATH}/torqueSim/console/typeValidators.cc
      ${KS_PATH}/torqueSim/sim/dynamicTypes.cc
      ${KS_PATH}/torqueSim/sim/scriptObject.cc
      ${KS_PATH}/torqueSim/sim/simBase.cc
      ${KS_PATH}/torqueSim/sim/simBase.h
      ${KS_PATH}/torqueSim/sim/simDictionary.cc
      ${KS_PATH}/torqueSim/sim/simDictionary.h
      ${KS_PATH}/torqueSim/sim/simFiberManager.cc
      ${KS_PATH}/torqueSim/sim/simFiberManager.h
      ${KS_PATH}/torqueSim/sim/simManager.cc
      # XXTH ${KS_PATH}/torqueSim/platform/basicPlatformProcess.cc
      ${KS_PATH}/torqueSim/platform/platformFileIO.cc
      ${KS_PATH}/torqueSim/platform/platformStrings.cc
      ${KS_PATH}/torqueSim/platform/platformThread.cc
      ${KS_PATH}/torqueSim/platform/platformNetAsync.cc
      ${KS_PATH}/torqueSim/platform/platformNetwork.cc
      ${KS_PATH}/torqueSim/platform/platformTelnet.cc
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
set(PROJECT_SOURCES
    ${KS_SRCS}
    ${TORQUESIM_SRCS}
    ${BOX2D_SRCS}
    # --- custom sources ---
    ${KORK_DIR}/src/main.cpp
    ${KORK_DIR}/src/appMain.h
    ${KORK_DIR}/src/appMain.cpp

    ${KORK_DIR}/src/myPlatformProcess.cc

    ${KORK_DIR}/src/b2b/b2btest.cc
    ${KORK_DIR}/src/b2b/b2btest.h
    ${KORK_DIR}/src/b2b/b2Objects.cc
    ${KORK_DIR}/src/b2b/b2Objects.h
    ${KORK_DIR}/src/b2b/b2Joints.cc
    ${KORK_DIR}/src/b2b/b2Joints.h

    ${KORK_DIR}/src/core/GameCtrl.h
    ${KORK_DIR}/src/core/GameCtrl.cpp
    ${KORK_DIR}/src/core/ConsoleTypes.cpp
    ${KORK_DIR}/src/core/ConsoleTypes.h
    ${KORK_DIR}/src/core/Grid.cpp
    ${KORK_DIR}/src/core/Grid.h

    ${KORK_DIR}/src/consoleFunctions/app.cpp
    ${KORK_DIR}/src/consoleFunctions/math.cpp

    ${KORK_DIR}/src/audio/AudioProfile.h
    ${KORK_DIR}/src/audio/AudioProfile.cpp

    ${KORK_DIR}/src/render/Camera.cpp
    ${KORK_DIR}/src/render/Camera.h
    ${KORK_DIR}/src/render/Font.h
    ${KORK_DIR}/src/render/Font.cpp
    ${KORK_DIR}/src/render/Label.h
    ${KORK_DIR}/src/render/Label.cpp
    ${KORK_DIR}/src/render/Light.cpp
    ${KORK_DIR}/src/render/Light.h
    ${KORK_DIR}/src/render/Particle.cpp
    ${KORK_DIR}/src/render/Particle.h
    ${KORK_DIR}/src/render/Sprite.h
    ${KORK_DIR}/src/render/Sprite.cpp
    ${KORK_DIR}/src/render/Texture.h
    ${KORK_DIR}/src/render/Texture.cpp

    ${KORK_DIR}/src/totp/base32.cc
    ${KORK_DIR}/src/totp/base32.h
    ${KORK_DIR}/src/totp/hmac-sha1.cc
    ${KORK_DIR}/src/totp/hmac-sha1.h
    ${KORK_DIR}/src/totp/totp.cc

    ${KORK_DIR}/src/math/mMathFn.h
    ${KORK_DIR}/src/math/math_ScriptBinding.cc
    # ${KORK_DIR}/src/math/mMath_C.cc

    # ${KORK_DIR}/src/network/
    # ${KORK_DIR}/src/network/tcpObject.cc
    # ${KORK_DIR}/src/network/tcpObject.h
    # ${KORK_DIR}/src/network/tcpObject_ScriptBinding.h


    # testing math from OGE/Torque2D
    # too many changes needed ... ${KORK_DIR}/src/math/mathTypes.cc


    # ---- editor
    ${KORK_DIR}/src/scriptEditor/scriptEditor.cpp
    ${KORK_DIR}/src/scriptEditor/scriptEditor.h
    ${KORK_DIR}/src/scriptEditor/ImGuiColorTextEdit/TextEditor.cpp
    ${KORK_DIR}/src/scriptEditor/ImGuiColorTextEdit/TextEditor.h

    # --- testing
    ${KORK_DIR}/src/test/testObj.cpp
)


configure_project_target(KorkTestBed ${KORK_DIR} "${KORK_DIR}/assets@assets" "${PROJECT_SOURCES}")

# target_compile_definitions(KorkTestBed PUBLIC TORQUE_ENABLE_ASSERTS)


target_include_directories(KorkTestBed PRIVATE ${KORK_DIR}/src)
target_include_directories(KorkTestBed PRIVATE ${KS_INCLUDES})

