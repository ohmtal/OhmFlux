//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#pragma once

#define TORQUE_SCRIPT_EXTENSION   "cs"
#define TORQUE_APP_NAME            "ElfTestBed"
#define TORQUE_APP_VERSION         1000
#define TORQUE_APP_VERSION_STRING  "26.06.06.0"
#define TORQUE_DISABLE_MEMORY_MANAGER
/* #undef TORQUE_DISABLE_VIRTUAL_MOUNT_SYSTEM */
/* #undef TORQUE_DISABLE_FIND_ROOT_WITHIN_ZIP */
/* #undef TORQUE_ZIP_DISK_LAYOUT */
/* #undef TORQUE_LOWER_ZIPCASE */
#define TORQUE_NO_DSO_GENERATION
/* #undef TORQUE_ENABLE_PROFILER */

// TORQUE_DEBUG is now set dynamically and not here anymore
// #define TORQUE_DEBUG
/* #undef TORQUE_SHIPPING */
/* #undef TORQUE_DEBUG_NET */
/* #undef TORQUE_DEBUG_NET_MOVES */
//#define MAXPACKETSIZE 1500
//#define TORQUE_DEBUG_GUARD
//#define TORQUE_ENABLE_THREAD_STATICS
//#define TORQUE_ENABLE_THREAD_STATIC_METRICS
//#define FRAMEALLOCATOR_DEBUG_GUARD

/// This #define is used by the FrameAllocator to set the size of the frame.
///
/// It was previously set to 3MB but I've increased it to 32MB due to the
/// FrameAllocator being used as temporary storage for bitmaps in the D3D9
/// texture manager.
#define TORQUE_FRAME_SIZE     32 << 20

