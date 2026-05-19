//-----------------------------------------------------------------------------
// Copyright (c) 2025-2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "platform/platform.h"
#include "platform/platformProcess.h"
#include "platform/platformFileIO.h"
#include "platform/threads/thread.h"
#include "platform/threads/mutex.h"
#include "platform/threads/semaphore.h"
#include "core/stringTable.h"
#include "core/safeDelete.h"

#include <mutex>
#include <string>

#ifdef TORQUE_USE_STD_FILESYSTEM
#include <filesystem>
namespace fs = std::filesystem;
#endif

namespace Platform
{

void init()
{
   
}

void process()
{
   
}

void shutdown()
{
   
}

void sleep(U32 ms)
{
   
}

void restartInstance()
{
   
}

void postQuitMessage(const U32 in_quitVal)
{
   
}

void forceShutdown(S32 returnValue)
{
   
}

StringTableEntry getUserHomeDirectory()
{
   return nullptr;
}

StringTableEntry getUserDataDirectory()
{
   return nullptr;
}


U32 getTime( void )
{
   return 0;
}

U32 getVirtualMilliseconds( void )
{
   return 0;
}

U32 getRealMilliseconds( void )
{
   return 0;
}

void advanceTime(U32 delta)
{
   
}

void getLocalTime(LocalTime &)
{
   
}

S32 compareFileTimes(const FileTime &a, const FileTime &b)
{
   return 0;
}

/// Math.
float getRandom()
{
   return 3;
}

void outputDebugString(const char *string)
{
   
}

/// File IO.
StringTableEntry getWorkingDirectory()
{
#ifdef TORQUE_USE_STD_FILESYSTEM
   static std::string sWorkingDirectory;

   std::error_code ec;
   fs::path cwd = fs::current_path(ec);
   if (ec)
      return nullptr;

   sWorkingDirectory = cwd.generic_string();
   return StringTable->insert(sWorkingDirectory.c_str());
#else
   return nullptr;
#endif
}

bool setWorkingDirectory(StringTableEntry newDir)
{
#ifdef TORQUE_USE_STD_FILESYSTEM
   if (newDir == nullptr || *newDir == '\0')
      return false;

   std::error_code ec;
   fs::current_path(fs::path(newDir), ec);
   return !ec;
#else
   (void)newDir;
   return false;
#endif
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
   return nullptr;
}

StringTableEntry getExecutablePath()
{
   return nullptr;
}

bool dumpPath(const char *in_pBasePath, std::vector<FileInfo>& out_rFileVector, S32 recurseDepth)
{
   return false;
}

bool dumpDirectories( const char *path, std::vector<StringTableEntry> &directoryVector, S32 depth, bool noBasePath )
{
   return false;
}

bool hasSubDirectory( const char *pPath )
{
   return false;
}

bool getFileTimes(const char *filePath, FileTime *createTime, FileTime *modifyTime)
{
   return false;
}

bool isFile(const char *pFilePath)
{
   return false;
}

S32  getFileSize(const char *pFilePath)
{
   return 0;
}

bool isDirectory(const char *pDirPath)
{
   return false;
}

bool isSubDirectory(const char *pParent, const char *pDir)
{
   return false;
}

bool createPath(const char *path)
{
   return false;
}

bool fileDelete(const char *name)
{
   return false;
}

bool fileRename(const char *oldName, const char *newName)
{
   return false;
}

bool fileTouch(const char *name)
{
   return false;
}

bool pathCopy(const char *fromName, const char *toName, bool nooverwrite)
{
   return false;
}

}
