//-----------------------------------------------------------------------------
// Copyright (c) 2025-2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// XXTH: Using SDL3 to start ....
// ORIG: torqueSim/platform/basicPlatformProcess.cc
//-----------------------------------------------------------------------------

#include <console/console.h>
#include "appMain.h"

namespace Platform
{
  void postQuitMessage(const S32 in_quitVal)
  {
    if (!ElfFlux::gMain) return;
    return ElfFlux::gMain->TerminateApplication();
  }

  void forceShutdown(S32 returnValue)
  {
    if (!ElfFlux::gMain) return;
    return ElfFlux::gMain->TerminateApplication();
  }


} //namespace
