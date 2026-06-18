//-----------------------------------------------------------------------------
// Copyright (c) 2025-2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// XXTH: Using SDL3 to start ....
// ORIG: torqueSim/platform/basicPlatformProcess.cc
//-----------------------------------------------------------------------------

#include <SDL3/SDL.h>
#include <console/console.h>
#include <string>
#include "core/fluxGlobals.h"
#include "appMain.h"
#include <core/volume.h>


StringTableEntry osGetTemporaryDirectory(){
   Con::warnf(" %s not implemented (%s:%d)", __func__, __FILE__, __LINE__);
   return StringTable->insert("");
}

namespace Platform
{
   //---------------------------------------------------------------------------
   bool isFile(const char *pFilePath) {
      if (!pFilePath) {
         return false;
      }
      SDL_PathInfo info;
      // Returns true on success, false on failure (e.g., path doesn't exist)
      if (SDL_GetPathInfo(pFilePath, &info)) {
         return (info.type == SDL_PATHTYPE_FILE);
      }

      return false;
   }
   //---------------------------------------------------------------------------
   U64 getTime( void )
   {
      SDL_Time nanoSeconds;
      if (SDL_GetCurrentTime(&nanoSeconds)) {
         return (U32)(nanoSeconds / SDL_NS_PER_SECOND);
      }
      Con::warnf("%s failed!", __func__);
      return 0;
   }
   //---------------------------------------------------------------------------
   U64 getRealMilliseconds( void )
   {
      SDL_Time nanoSeconds;
      if (SDL_GetCurrentTime(&nanoSeconds)) {
         return (U32)(nanoSeconds / SDL_NS_PER_MS);
      }
      Con::warnf("%s failed!", __func__);
      return 0;
   }

   //---------------------------------------------------------------------------
   //---------------------------------------------------------------------------
   //TODO: ... lot of ... just in time :P ...
   //---------------------------------------------------------------------------
   void init()
{
   Con::warnf("%s not implemented", __func__);
}

void process()
{
   Con::warnf("%s not implemented", __func__);
}

void shutdown()
{
   Con::warnf("%s not implemented", __func__);
}

void sleep(U32 ms)
{
   Con::warnf("%s not implemented", __func__);
}

void restartInstance()
{
   Con::warnf("%s not implemented", __func__);
}

void postQuitMessage(const S32 in_quitVal)
{
  // SDL_Quit();
  if (!ElfFlux::gMain) return;
  return ElfFlux::gMain->TerminateApplication();
}

void forceShutdown(S32 returnValue)
{
   // SDL_Quit();
   if (!ElfFlux::gMain) return;
    return ElfFlux::gMain->TerminateApplication();
}

StringTableEntry getUserHomeDirectory()
{
   Con::warnf("%s not implemented", __func__);
   return nullptr;
}

StringTableEntry getUserDataDirectory()
{
   Con::warnf("%s not implemented", __func__);
   return nullptr;
}



U64 getVirtualMilliseconds( void )
{
   Con::warnf("%s not implemented", __func__);
   return 0;
}


void advanceTime(U32 delta)
{
   Con::warnf("%s not implemented", __func__);
}

void getLocalTime(LocalTime &)
{
   Con::warnf("%s not implemented", __func__);
}
String localTimeToString(Platform::LocalTime const&) {
   Con::warnf(" %s not implemented (%s:%d)", __func__, __FILE__, __LINE__);
   return "";
}

S32 compareFileTimes(const FileTime &a, const FileTime &b)
{
   Con::warnf("%s not implemented", __func__);
   return 0;
}

/// Math.
float getRandom()
{
   Con::warnf("%s not implemented", __func__);
   return 3;
}

void outputDebugString(const char *string)
{
   Con::warnf("%s not implemented", __func__);
}
void outputDebugString(const char *string, ...) {
   Con::warnf(" %s not implemented (%s:%d)", __func__, __FILE__, __LINE__);

}
/// File IO.

// StringTableEntry getWorkingDirectory()
// {
//
// }

// bool setWorkingDirectory(StringTableEntry newDir)
// {
//   if (newDir)  {
//     gWorkingDirectory = newDir;
//     return true;
//   }
//   return false;
// }

// StringTableEntry getCurrentDirectory()
// {
//    return Torque::FS::GetCwd().getPath();
// }
//
// bool setCurrentDirectory(StringTableEntry newDir)
// {
//    return Torque::FS::SetCwd(newDir);
// }

StringTableEntry getExecutableName()
{
   Con::warnf("%s not implemented", __func__);
   return StringTable->insert("");
}

StringTableEntry getExecutablePath()
{
   return StringTable->insert(SDL_GetBasePath());
}


bool dumpPath(const char *in_pBasePath, Vector<FileInfo>& out_rFileVector, S32 recurseDepth )
{
   Con::warnf(" %s not implemented (%s:%d)", __func__, __FILE__, __LINE__);
   return false;
}

bool dumpDirectories( const char *path, Vector<StringTableEntry> &directoryVector, S32 depth, bool noBasePath )
{
   Con::warnf(" %s not implemented (%s:%d)", __func__, __FILE__, __LINE__);
   return false;
}

bool hasSubDirectory( const char *pPath )
{
   Con::warnf("%s not implemented", __func__);
   return false;
}

bool getFileTimes(const char *filePath, FileTime *createTime, FileTime *modifyTime)
{
   Con::warnf("%s not implemented", __func__);
   return false;
}


S32  getFileSize(const char *pFilePath)
{
   Con::warnf("%s not implemented", __func__);
   return 0;

}

bool isDirectory(const char *pDirPath)
{
   Con::warnf("%s not implemented", __func__);
   return false;
}

bool isSubDirectory(const char *pParent, const char *pDir)
{
   Con::warnf("%s not implemented", __func__);
   return false;
}

bool createPath(const char *path)
{
   Con::warnf("%s not implemented", __func__);
   return false;
}

bool fileDelete(const char *name)
{
   Con::warnf("%s not implemented", __func__);
   return false;
}

bool fileRename(const char *oldName, const char *newName)
{
   Con::warnf("%s not implemented", __func__);
   return false;
}

bool fileTouch(const char *name)
{
   Con::warnf("%s not implemented", __func__);
   return false;
}

bool pathCopy(const char *fromName, const char *toName, bool nooverwrite)
{
   Con::warnf("%s not implemented", __func__);
   return false;
}

void debugBreak()
{
   Con::warnf(" %s not implemented (%s:%d)", __func__, __FILE__, __LINE__);
}
bool setClipboard(const char* text)
{
   Con::warnf(" %s not implemented (%s:%d)", __func__, __FILE__, __LINE__);
   return false;
}

const char* getClipboard() {
   Con::warnf(" %s not implemented (%s:%d)", __func__, __FILE__, __LINE__);
   return nullptr;
}

void AlertOK(const char *windowTitle, const char *message) {
   Con::warnf(" %s not implemented (%s:%d)", __func__, __FILE__, __LINE__);

}
bool AlertOKCancel(const char *windowTitle, const char *message) {
   Con::warnf(" %s not implemented (%s:%d)", __func__, __FILE__, __LINE__);
   return false;
}
bool AlertRetry(const char *windowTitle, const char *message) {
   Con::warnf(" %s not implemented (%s:%d)", __func__, __FILE__, __LINE__);
   return false;
}
ALERT_ASSERT_RESULT AlertAssert(const char *windowTitle, const char *message)
{
   Con::warnf(" %s not implemented (%s:%d)", __func__, __FILE__, __LINE__);
   return ALERT_ASSERT_IGNORE;
}

} //namespace
