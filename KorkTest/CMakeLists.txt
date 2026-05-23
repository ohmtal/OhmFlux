set(KORK_DIR "${CMAKE_CURRENT_LIST_DIR}")

#--------------------------

# ----------------- korkscript Test
option(ENABLE_KS "Enable korkscript" ON)

if (true #[[ENABLE_KS]])
  set (KS_PATH "${KORK_DIR}/lib/korkscript")

  find_package(Git REQUIRED)

  # Check if the submodule's source files exist
  if(NOT EXISTS "${$KS_PATH}/CMakeLists.txt" )
      message(STATUS "Initializing Git submodules...")
      execute_process(
          COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          RESULT_VARIABLE GIT_SUBMODULE_RESULT
      )

      if(NOT GIT_SUBMODULE_RESULT EQUAL 0)
          message(FATAL_ERROR "git submodule update failed with code ${GIT_SUBMODULE_RESULT}")
      endif()
  endif()



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



# -------------------
set(PROJECT_SOURCES
    ${KS_SRCS}
    ${TORQUESIM_SRCS}
    # --- custom sources ---
    ${KORK_DIR}/src/main.cpp
    ${KORK_DIR}/src/appMain.h
    ${KORK_DIR}/src/appMain.cpp

    ${KORK_DIR}/src/myPlatformProcess.cc


    ${KORK_DIR}/src/core/GameCtrl.h
    ${KORK_DIR}/src/core/GameCtrl.cpp


    ${KORK_DIR}/src/consoleFunctions/app.cpp
    ${KORK_DIR}/src/consoleFunctions/math.cpp

    ${KORK_DIR}/src/audio/AudioProfile.h
    ${KORK_DIR}/src/audio/AudioProfile.cpp


    ${KORK_DIR}/src/render/Texture.h
    ${KORK_DIR}/src/render/Texture.cpp
    ${KORK_DIR}/src/render/Sprite.h
    ${KORK_DIR}/src/render/Sprite.cpp


    ${KORK_DIR}/src/totp/base32.cc
    ${KORK_DIR}/src/totp/base32.h
    ${KORK_DIR}/src/totp/hmac-sha1.cc
    ${KORK_DIR}/src/totp/hmac-sha1.h
    ${KORK_DIR}/src/totp/totp.cc

    # testing math from OGE/Torque2D
    # too many changes needed ... ${KORK_DIR}/src/math/mathTypes.cc

)


configure_project_target(KorkTestBed ${KORK_DIR} "${KORK_DIR}/assets@assets" "${PROJECT_SOURCES}")
target_include_directories(KorkTestBed PRIVATE ${KORK_DIR}/src)
target_include_directories(KorkTestBed PRIVATE ${KS_INCLUDES})
