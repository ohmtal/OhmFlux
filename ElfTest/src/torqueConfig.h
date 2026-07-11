//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

// ----------------------------------------------------------------------------
// make slots type save but same slotname always have the same type

// enable #define in torque script
#define ELFSCRIPT_PREPROCESSOR

// Autodelete objects on shutdown - this can slowdown delete on many objects
#define ELFSCRIPT_GARBAGECOLLECTION

// fields are stored in a map to save fieldtype on static fields
// #define ELFSCRIPT_STRICT_SLOT_TYPE

// cache function calls WARNING: inconsitent
// #define ELFSCRIPT_CALLFUNC_CACHED

// Fast Path static fields float/interger
#define ELFSCRIPT_FASTPATH_FLD

// ----------------------------------------------------------------------------
#ifdef FLUX_DEBUG
#define TORQUE_DEBUG
#define TORQUE_ENABLE_ASSERTS
#endif
//no idea what "new" means ;) but i try it out
// #define USE_NEW_SIMDICTIONARY << not woking "objectName" missing


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

