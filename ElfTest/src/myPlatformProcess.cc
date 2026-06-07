//-----------------------------------------------------------------------------
// Copyright (c) 2025-2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// XXTH: Using SDL3 to start ....
// ORIG: torqueSim/platform/basicPlatformProcess.cc
//-----------------------------------------------------------------------------

// ~~~ 1. remove unused includes... ~~~
// #include <stdio.h>
// #include <stdlib.h>
// #include "platform/platform.h"
// #include "platform/platformProcess.h"
// #include "platform/platformFileIO.h"
// #include "platform/threads/thread.h"
// #include "platform/threads/mutex.h"
// #include "platform/threads/semaphore.h"
// #include "core/stringTable.h"
// #include "core/safeDelete.h"
//
// #include <mutex>
// #include <string>
//
// #ifdef TORQUE_USE_STD_FILESYSTEM
// #include <filesystem>
// namespace fs = std::filesystem;
// #endif

// ~~~ 1. add includes... ~~~
#include <SDL3/SDL.h>
#include <console/console.h>
#include <platform/platformProcess.h>


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
   // FIXME U32 => U64!
   U32 getTime( void )
   {
      SDL_Time nanoSeconds;
      if (SDL_GetCurrentTime(&nanoSeconds)) {
         return (U32)(nanoSeconds / SDL_NS_PER_SECOND);
      }
      Con::warnf("%s failed!", __func__);
      return 0;
   }
   //---------------------------------------------------------------------------
   // FIXME U32 => U64!
   U32 getRealMilliseconds( void )
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

void postQuitMessage(const U32 in_quitVal)
{
   Con::warnf("%s not implemented", __func__);
}

void forceShutdown(S32 returnValue)
{
   Con::warnf("%s not implemented", __func__);
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


// FIXME U64!
U32 getVirtualMilliseconds( void )
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

/// File IO.
StringTableEntry getWorkingDirectory()
{
   Con::warnf("%s not implemented", __func__);
   return nullptr;
}

bool setWorkingDirectory(StringTableEntry newDir)
{
   Con::warnf("%s not implemented", __func__);
   (void)newDir;
   return false;

}

StringTableEntry getCurrentDirectory()
{
   return getWorkingDirectory();
}

bool setCurrentDirectory(StringTableEntry newDir)
{
   return setWorkingDirectory(newDir);
}

StringTableEntry getExecutableName()
{
   Con::warnf("%s not implemented", __func__);
   return nullptr;
}

StringTableEntry getExecutablePath()
{
   Con::warnf("%s not implemented", __func__);
   return nullptr;
}

bool dumpPath(const char *in_pBasePath, std::vector<FileInfo>& out_rFileVector, S32 recurseDepth)
{
   Con::warnf("%s not implemented", __func__);
   return false;
}

bool dumpDirectories( const char *path, std::vector<StringTableEntry> &directoryVector, S32 depth, bool noBasePath )
{
   Con::warnf("%s not implemented", __func__);
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

}
