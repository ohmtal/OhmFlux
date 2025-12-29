//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//  << dangling pointer >> when object is deleted before action is fired ==>
//      Added owner and cleanByOwner in FluxBaseObject to manage this
//
// Example usage:
// =============
//
// ****** Scheduler test on deleted object ***** >>>
myFish* lFish = static_cast<myFish*>(hitObj);
// it call peep after the fish is deleted or better not ;)
FluxSchedule.add(2.0, lFish, [lFish]() {
    lFish->peep();
});
// test "global schedule still works" sending the current fps to it
FluxSchedule.add(2.0, nullptr, [savedFPS = getFPS()]() {
    _MainPeep_(savedFPS);
});
// <<<
// //-----------------------------------------------------------------------------

#pragma once
#include <vector>
#include <functional>
#include <mutex>
#include <algorithm>
//-----------------------------------------------------------------------------
// Global shorthand macro
#define FluxSchedule FluxScheduler::get()
// Forward:
class FluxBaseObject;
//-----------------------------------------------------------------------------
class FluxScheduler
{
public:
    using TaskID = size_t;

private:
    FluxScheduler() : mNextId(0) {}
    ~FluxScheduler() = default;

    // Delete copy/move to enforce Singleton
    FluxScheduler(const FluxScheduler&) = delete;
    void operator=(const FluxScheduler&) = delete;

    struct ScheduledTask {
        TaskID id;
        double timeRemaining;
        std::function<void()> action;
        void* owner;
    };

    std::vector<ScheduledTask> mTasks;
    TaskID mNextId;

    // IMPORTANT: Recursive mutex allows a task to call .add()
    // or .cancel() without deadlocking the thread.
    std::recursive_mutex mMutex;

    bool mIsShutdown = false;

public:

    //-------------------------------------------------------------------------
    // Meyers' Singleton: Thread-safe initialization
    static FluxScheduler& get();

    // Adds a task and returns a unique ID
    // TaskID add(double lDelaySeconds, void* lOwner, std::function<void()> lAction);
    TaskID add(double lDelaySeconds, FluxBaseObject* lOwner, std::function<void()> lAction);

    // Check if a task is still waiting
    bool isPending(TaskID id);

    // Cancel a specific task
    void cancel(TaskID id);

    // Wipe all tasks (e.g., during level loading)
    void clear();

    //update and launch actions
    void update(double dt);

    //shutdown and clean up
    void shutdown();

    // clean by a Owner to prevent pointer dangling
    void cleanByOwner(void* lOwner);

};
